/*
 * libjingle
 * Copyright 2004--2005, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "talk/examples/iphone/presencepushtask.h"

#include "talk/base/stringencode.h"
#include "talk/examples/iphone/muc.h"
#include "talk/xmpp/constants.h"



namespace buzz {

// string helper functions -----------------------------------------------------

static bool
IsXmlSpace(int ch) {
  return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t';
}

static bool ListContainsToken(const std::string & list,
                              const std::string & token) {
  size_t i = list.find(token);
  if (i == std::string::npos || token.empty())
    return false;
  bool boundary_before = (i == 0 || IsXmlSpace(list[i - 1]));
  bool boundary_after = (i == list.length() - token.length() ||
                         IsXmlSpace(list[i + token.length()]));
  return boundary_before && boundary_after;
}


bool PresencePushTask::HandleStanza(const XmlElement * stanza) {
  if (stanza->Name() != QN_PRESENCE)
    return false;
  QueueStanza(stanza);
  return true;
}

static bool IsUtf8FirstByte(int c) {
  return (((c)&0x80)==0) || // is single byte
    ((unsigned char)((c)-0xc0)<0x3e); // or is lead byte
}

int PresencePushTask::ProcessStart() {
  const XmlElement * stanza = NextStanza();
  if (stanza == NULL)
    return STATE_BLOCKED;

  Jid from(stanza->Attr(QN_FROM));
  std::map<Jid, buzz::Muc*>::const_iterator elem =
      client_->mucs().find(from.BareJid());
  if (elem == client_->mucs().end()) {
    HandlePresence(from, stanza);
  } else {
    HandleMucPresence(elem->second, from, stanza);
  }

  return STATE_START;
}

void PresencePushTask::HandlePresence(const Jid& from,
                                      const XmlElement* stanza) {
  if (stanza->Attr(QN_TYPE) == STR_ERROR)
    return;

  Status s;
  FillStatus(from, stanza, &s);
  SignalStatusUpdate(s);
}

void PresencePushTask::HandleMucPresence(buzz::Muc* muc,
                                         const Jid& from,
                                         const XmlElement* stanza) {
  if (from == muc->local_jid()) {
    if (!stanza->HasAttr(QN_TYPE)) {
      // We joined the MUC.
      const XmlElement* elem = stanza->FirstNamed(QN_MUC_USER_X);
      if (elem) {
        elem = elem->FirstNamed(QN_MUC_USER_STATUS);
      }
      if (elem && (elem->Attr(QN_CODE) == "110" ||
          elem->Attr(QN_CODE) == "100")) {
        SignalMucJoined(muc->jid());
      }
    } else {
      // We've been kicked. Bye.
      int error = 0;
      if (stanza->Attr(QN_TYPE) == STR_ERROR) {
        const XmlElement* elem = stanza->FirstNamed(QN_ERROR);
        if (elem && elem->HasAttr(QN_CODE)) {
          error = atoi(elem->Attr(QN_CODE).c_str());
        }
      }
      SignalMucLeft(muc->jid(), error);
    }
  } else {
    MucStatus s;
    FillMucStatus(from, stanza, &s);
    SignalMucStatusUpdate(muc->jid(), s);
  }
}

void PresencePushTask::FillStatus(const Jid& from, const XmlElement* stanza,
                                  Status* s) {
  s->set_jid(from);
  if (stanza->Attr(QN_TYPE) == STR_UNAVAILABLE) {
    s->set_available(false);
  } else {
    s->set_available(true);
    const XmlElement * status = stanza->FirstNamed(QN_STATUS);
    if (status != NULL) {
      s->set_status(status->BodyText());

      // Truncate status messages longer than 300 bytes
      if (s->status().length() > 300) {
        size_t len = 300;

        // Be careful not to split legal utf-8 chars in half
        while (!IsUtf8FirstByte(s->status()[len]) && len > 0) {
          len -= 1;
        }
        std::string truncated(s->status(), 0, len);
        s->set_status(truncated);
      }
    }

    const XmlElement * priority = stanza->FirstNamed(QN_PRIORITY);
    if (priority != NULL) {
      int pri;
      if (talk_base::FromString(priority->BodyText(), &pri)) {
        s->set_priority(pri);
      }
    }

    const XmlElement * show = stanza->FirstNamed(QN_SHOW);
    if (show == NULL || show->FirstChild() == NULL) {
      s->set_show(Status::SHOW_ONLINE);
    }
    else {
      if (show->BodyText() == "away") {
        s->set_show(Status::SHOW_AWAY);
      }
      else if (show->BodyText() == "xa") {
        s->set_show(Status::SHOW_XA);
      }
      else if (show->BodyText() == "dnd") {
        s->set_show(Status::SHOW_DND);
      }
      else if (show->BodyText() == "chat") {
        s->set_show(Status::SHOW_CHAT);
      }
      else {
        s->set_show(Status::SHOW_ONLINE);
      }
    }

    const XmlElement * caps = stanza->FirstNamed(QN_CAPS_C);
    if (caps != NULL) {
      std::string node = caps->Attr(QN_NODE);
      std::string ver = caps->Attr(QN_VER);
      std::string exts = caps->Attr(QN_EXT);

      s->set_know_capabilities(true);

      if (node == GOOGLE_CLIENT_NODE) {
        s->set_is_google_client(true);
        s->set_version(ver);
      }

      if (ListContainsToken(exts, "voice-v1")) {
        s->set_phone_capability(true);
      }
      if (ListContainsToken(exts, "video-v1")) {
        s->set_video_capability(true);
      }
    }

    const XmlElement* delay = stanza->FirstNamed(kQnDelayX);
    if (delay != NULL) {
      // Ideally we would parse this according to the Psuedo ISO-8601 rules
      // that are laid out in JEP-0082:
      // http://www.jabber.org/jeps/jep-0082.html
      std::string stamp = delay->Attr(kQnStamp);
      s->set_sent_time(stamp);
    }
  }
}

void PresencePushTask::FillMucStatus(const Jid& from, const XmlElement* stanza,
                                     MucStatus* s) {
  // First get the normal user status info. Happily, this is in the same
  // format as it is for user presence.
  FillStatus(from, stanza, s);

  // Now look for src IDs, which will be present if this user is in a
  // multiway call to this MUC.
  const XmlElement* xstanza = stanza->FirstNamed(QN_MUC_USER_X);
  if (xstanza) {
    const XmlElement* media;
    for (media = xstanza->FirstNamed(QN_GOOGLE_MUC_USER_MEDIA);
        media; media = media->NextNamed(QN_GOOGLE_MUC_USER_MEDIA)) {

      const XmlElement* type = media->FirstNamed(QN_GOOGLE_MUC_USER_TYPE);
      if (!type) continue; // Shouldn't happen

      const XmlElement* src_id = media->FirstNamed(QN_GOOGLE_MUC_USER_SRC_ID);
      if (!src_id) continue; // Shouldn't happen

      char *endptr;
      uint32 src_id_num = strtoul(src_id->BodyText().c_str(), &endptr, 10);
      if (src_id->BodyText().c_str()[0] == '\0' || endptr[0] != '\0') {
        // String is not composed exclusively of leading whitespace plus a
        // number (shouldn't happen). Ignore it.
        continue;
      }
      // Else it's valid. Set it.

      if (type->BodyText() == "audio") {
        // This is the audio media element. Get the src-id.
        s->set_audio_src_id(src_id_num);
      } else if (type->BodyText() == "video") {
        // This is the video media element. Get the src-id.
        s->set_video_src_id(src_id_num);
      }
    }
  }
}

}
