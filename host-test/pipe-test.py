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

class TestHostPipe(unittest.TestCase):

  def get_response(self):
      response_length = struct.unpack("=I", self.p.stdout.read(4))[0]
      response = str(self.p.stdout.read(response_length))
      # make it into "oneline" json before printing
      response_print = json.dumps(json.loads(response))
      print("RECV: %s" % response_print)
      return json.loads(response)

  def transceive(self, msg):
      # send like described in 
      print("SEND: %s" % msg)
      self.p.stdin.write(struct.pack("=I", len(msg)))
      self.p.stdin.write(msg)
      # now read the input
      return self.get_response()

  def setUp(self):
      should_close_fds = sys.platform.startswith('win32') == False;
      self.p = subprocess.Popen(testconf.get_exe(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=should_close_fds, stderr=None)
      print("\nRunning native component on PID %d" % self.p.pid)

  def tearDown(self):
      if self.p.poll() == None:
          self.p.terminate()

  def test_random_string(self):
      cmd = "BLAH"
      resp = self.transceive(cmd)
      self.assertEquals(resp["result"], "invalid_argument")
      self.assertEqual(self.p.wait(), 1)

  def test_plain_string(self):
      self.p.stdin.write("Hello World!")
      resp = self.get_response()
      self.assertEquals(resp["result"], "invalid_argument")
      self.assertEqual(self.p.wait(), 1)

  def test_empty_json(self):
      cmd = {}
      resp = self.transceive(json.dumps(cmd))
      self.assertEqual(resp["result"], "invalid_argument")
      self.assertEqual(self.p.wait(), 1)

  def test_utopic_length(self):
      # write big bumber and little data
      self.p.stdin.write(struct.pack("=I", 0xFFFFFFFF))
      self.p.stdin.write("Hello World!")
      resp = self.get_response()
      self.assertEquals(resp["result"], "invalid_argument")
      self.assertEqual(self.p.wait(), 1)

#  def test_length_exceeds_data(self):
#      # write length > data size
#      self.p.stdin.write(struct.pack("=I", 0x0000000F))
#      self.p.stdin.write("Hello World!")
#      resp = self.get_response()
#      self.assertEquals(resp["result"], "invalid_argument")
#      self.assertEqual(self.p.wait(), 1)

  def test_inconsistent_origin(self):
      cmd = {"type": "VERSION", "nonce": str(uuid.uuid4()), "origin": "http://example.com/"}
      cmd2 = {"type": "VERSION", "nonce": str(uuid.uuid4()), "origin": "http://badexample.com/"}
      resp1 = self.transceive(json.dumps(cmd))
      self.assertEqual(resp1["result"], "ok")
      resp2 = self.transceive(json.dumps(cmd2))
      self.assertEqual(resp2["result"], "invalid_argument")

  # other headless tests
  def test_version_no_nonce(self):
      cmd = {"type": "VERSION", "origin": "https://example.com/"}
      resp = self.transceive(json.dumps(cmd))
      self.assertEqual(resp["result"], "invalid_argument")

  def test_version_no_origin(self):
      cmd = {"type": "VERSION", "nonce": str(uuid.uuid4())}
      resp = self.transceive(json.dumps(cmd))
      self.assertEqual(resp["result"], "invalid_argument")

  def test_version_invalid_origin(self):
      cmd = {"type": "VERSION", "nonce": str(uuid.uuid4()), "origin": "foobar in da house"}
      resp = self.transceive(json.dumps(cmd))
      self.assertEqual(resp["result"], "ok")
      self.assertTrue(re.compile("^\d\.\d+\.\d+\.\d{1,3}$").match(resp["version"]))

  def test_version_file_origin(self):
      cmd = {"type": "VERSION", "nonce": str(uuid.uuid4()), "origin": "file:///tmp/index.html"}
      resp = self.transceive(json.dumps(cmd))
      self.assertEqual(resp["result"], "ok")
      self.assertTrue(re.compile("^\d\.\d+\.\d+\.\d{1,3}$").match(resp["version"]))

  def test_version_http_origin(self):
      cmd = {"type": "VERSION", "nonce": str(uuid.uuid4()), "origin": "http://example.com/"}
      resp = self.transceive(json.dumps(cmd))
      self.assertEqual(resp["result"], "ok")
      self.assertTrue(re.compile("^\d\.\d+\.\d+\.\d{1,3}$").match(resp["version"]))

  def test_version_https(self):
      cmd = {"type": "VERSION", "nonce": str(uuid.uuid4()), "origin": "https://example.com/"}
      resp = self.transceive(json.dumps(cmd))
      self.assertEqual(resp["result"], "ok")
      self.assertTrue(re.compile("^\d\.\d+\.\d+\.\d{1,3}$").match(resp["version"]))


if __name__ == '__main__':
    # run tests
    unittest.main()
