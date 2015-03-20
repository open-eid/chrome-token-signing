#
# Chrome Token Signing Native Host
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#

import json
import subprocess
import struct
import sys
import unittest
import uuid
import re
import testconf

# The protocol datagram is described here:
# https://developer.chrome.com/extensions/nativeMessaging#native-messaging-host-protocol
#
# The protocol itself is described here:
# https://github.com/open-eid/chrome-token-signing/wiki/NativeMessagingAPI


def instruct(msg):
    raw_input('>>>>>> %s\n[press ENTER to continue]' % msg)

class TestStatelessHost(unittest.TestCase):

  def open_conn(self):
      should_close_fds = sys.platform.startswith('win32') == False;
      self.p = subprocess.Popen(testconf.get_exe(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=should_close_fds, stderr=None)
      print ("Running native component on PID %d" % self.p.pid)
  
  def close_conn(self):
      self.p.wait()
  
  def fresh_connection(self):
      self.close_conn()
      self.open_conn()

  def transceive(self, msg):
      # send like described in 
      print ("SEND: %s" % msg)
      self.p.stdin.write(struct.pack("=I", len(msg)))
      self.p.stdin.write(msg)
      # now read the input
      response_length = struct.unpack("=I", self.p.stdout.read(4))[0]
      response = str(self.p.stdout.read(response_length))
      # make it into "oneline" json before printing
      response_print = json.dumps(json.loads(response))
      print ("RECV: %s" % response_print)
      return json.loads(response)

  def complete_msg(self, msg):
      msg["nonce"] = str(uuid.uuid4())
      msg["lang"] = "en"
      msg["origin"] = "https://example.com/test"
      return msg
  
  def setUp(self):
      self.open_conn()
  
  def tearDown(self):
      self.close_conn()

  
  def test_random_string(self):
      cmd = "BLAH"
      resp = self.transceive(cmd)
      self.assertEquals(resp["result"], "invalid_argument")
  
  def test_utopic_length(self):
      self.p.stdin.write(struct.pack("=I", 0xFFFFFFFE))
      # response_length = struct.unpack("=I", self.p.stdout.read(4))[0]
  
  def test_nonce_echo(self):
      cmd = self.complete_msg({"type": "VERSION"})
      original_nonce = cmd["nonce"]
      resp = self.transceive(json.dumps(cmd))
      self.assertEqual(resp["nonce"], original_nonce)

  def test_version(self):
      cmd = json.dumps(self.complete_msg({"type":"VERSION"}))
      resp = self.transceive(cmd)
      self.assertTrue(resp["version"] == "LOCAL_BUILD" or re.compile("^\d\.\d+\.\d+\.\d{1,3}$").match(resp["version"]))
  
  def test_get_certificate_cancel(self):
      instruct('Insert card and press CANCEL in dialog')
      cmd = json.dumps(self.complete_msg({"type":"CERT"}))
      resp = self.transceive(cmd)
      self.assertEquals(resp["result"], "user_cancel")

  def test_get_certificate_error(self):
      instruct('Insert card and try to REMOVE it while reading the certificate')
      cmd = json.dumps(self.complete_msg({"type":"CERT"}))
      resp = self.transceive(cmd)
      self.assertEquals(resp["result"], "technical_error")

  def test_get_certificate_ok(self):
      instruct('Insert card and select certificate')
      cmd = json.dumps(self.complete_msg({"type":"CERT"}))
      resp = self.transceive(cmd)
      self.assertEquals(resp["result"], "ok")

  def test_get_certificate_none(self):
      instruct('Remove card from reader')
      cmd = json.dumps(self.complete_msg({"type":"CERT"}))
      resp = self.transceive(cmd)
      self.assertEquals(resp["result"], "no_certificates")

  def test_get_certificate_no_reader(self):
      instruct('Remove reader')
      cmd = json.dumps(self.complete_msg({"type":"CERT"}))
      resp = self.transceive(cmd)
      self.assertEquals(resp["result"], "no_certificates")

  def test_get_certificate_and_sign_ok(self):
      instruct('Insert card, select certificate and sign successfully')
      cmd = json.dumps(self.complete_msg({"type":"CERT"}))
      resp = self.transceive(cmd)
      self.assertEquals(resp["result"], "ok")
      self.assertTrue(len(resp["cert"]) > 100)
      # hack for stateless operation
      self.close_conn()
      self.open_conn()
      cmd2 = json.dumps(self.complete_msg({"type":"SIGN", "hash": "0102030405060708090a0b0c0d0e0f0102030405", "cert": resp["cert"]}))
      resp2 = self.transceive(cmd2)
      self.assertEquals(resp2["result"], "ok")
      self.assertTrue("signature" in resp2)


  def test_get_certificate_and_sign_badhash(self):
      instruct('Insert card, select certificate')
      cmd = json.dumps(self.complete_msg({"type":"CERT"}))
      resp = self.transceive(cmd)
      self.assertEquals(resp["result"], "ok")
      self.assertTrue(len(resp["cert"]) > 100)
      # hack for stateless operation
      self.fresh_connection()
      cmd2 = json.dumps(self.complete_msg({"type":"SIGN", "hash": "0102030405060708090a0b0c0d0e0f010203", "cert": resp["cert"]}))
      resp2 = self.transceive(cmd2)
      self.assertEquals(resp2["result"], "invalid_argument")

      
      

if __name__ == '__main__':
    # run tests
    unittest.main();
    
