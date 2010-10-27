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

#include <time.h>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <vector>
#include "talk/base/logging.h"
#include "talk/base/flags.h"
#include "talk/base/pathutils.h"
#include "talk/base/stream.h"
#include "talk/base/ssladapter.h"
#include "talk/base/win32socketserver.h"
#include "talk/xmpp/xmppclientsettings.h"
#include "talk/examples/login/xmppthread.h"
#include "talk/examples/login/xmppauth.h"
#include "talk/examples/login/xmpppump.h"
#include "talk/examples/iphone/callclient.h"
//#include "talk/examples/iphone/console.h"

static const int DEFAULT_PORT = 5222;

int _main() {
  // This app has three threads. The main thread will run the XMPP client,
  // which will print to the screen in its own thread. A second thread
  // will get input from the console, parse it, and pass the appropriate
  // message back to the XMPP client's thread. A third thread is used
  // by MediaSessionClient as its worker thread.

  buzz::Jid jid;
  std::string username;
  talk_base::InsecureCryptStringImpl pass;

  username = "";
  if (username.find('@') == std::string::npos) {
    username.append("@localhost");
  }
  jid = buzz::Jid(username);
  if (!jid.IsValid() || jid.node() == "") {
    printf("Invalid JID. JIDs should be in the form user@domain\n");
    return 1;
  }
  pass.password() = "";

  buzz::XmppClientSettings xcs;
  xcs.set_user(jid.node());
  xcs.set_resource("call");
  xcs.set_host(jid.domain());
  xcs.set_pass(talk_base::CryptString(pass));
  xcs.set_server(talk_base::SocketAddress("talk.google.com", DEFAULT_PORT/*host, port*/));
  //printf("Logging in to %s as %s\n", server.c_str(), jid.Str().c_str());

  talk_base::InitializeSSL();

#if WIN32
  // Need to pump messages on our main thread on Windows.
  talk_base::Win32Thread w32_thread;
  talk_base::ThreadManager::SetCurrent(&w32_thread);
#endif
	talk_base::Thread athread;
	talk_base::ThreadManager::SetCurrent(&athread);

  talk_base::Thread* main_thread = talk_base::Thread::Current();
  assert(main_thread!=NULL);

  XmppPump pump;
  CallClient *client = new CallClient(pump.client());

  pump.DoLogin(xcs, new XmppSocket(true), NULL);
  main_thread->Run();
	
	// stop it
  //pump.DoDisconnect();

  //console->Stop();
  //delete console;
  //delete client;

  return 0;
}
