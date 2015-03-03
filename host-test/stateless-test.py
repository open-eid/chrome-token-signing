import json
import subprocess
import struct
import sys
import unittest
import uuid

exe = None

# The protocol datagram is described here:
# https://developer.chrome.com/extensions/nativeMessaging#native-messaging-host-protocol
#
# The protocol itself is described here:
# https://github.com/open-eid/chrome-token-signing/wiki/NativeMessagingAPI


class TestSequenceFunctions(unittest.TestCase):

  def tranceive(self, msg):
    # send like described in 
    print "SEND: %s" % msg
    self.p.stdin.write(struct.pack("=I", len(msg)))
    self.p.stdin.write(msg)
    # now read the input
    response_length = struct.unpack("=I", self.p.stdout.read(4))[0]
    response = str(self.p.stdout.read(response_length))
    # make it into "oneline" json before printing
    response_print = json.dumps(json.loads(response))
    print "RECV: %s" % response_print
    return json.loads(response)

  def complete_msg(self, msg):
      msg["nonce"] = str(uuid.uuid4())
      msg["lang"] = "eng"
      msg["protocol"] = "https:"
      return msg
  
  def setUp(self):
      global exe
      self.p = subprocess.Popen(exe, stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True, stderr=None)
      print "Running native component on PID %d" % self.p.pid
  
  def test_random_string(self):
      cmd = "BLAH"
      self.tranceive(cmd)
  
  def test_utopic_length(self):
      self.p.stdin.write(struct.pack("=I", 0xFFFFFFFE))
      response_length = struct.unpack("=I", self.p.stdout.read(4))[0]    
  
  def test_nonce_echo(self):
      cmd = self.complete_msg({"type": "VERSION"})
      original_nonce = cmd["nonce"]
      resp = self.tranceive(json.dumps(cmd))
      self.assertEqual(resp["nonce"], original_nonce)

  def test_version(self):
      cmd = json.dumps(self.complete_msg({"type":"VERSION"}))
      resp = self.tranceive(cmd)
      self.assertEqual(resp["version"], "LOCAL_BUILD")
  
  def test_get_certificate_cancel(self):
      print "PRESS CANCEL IN THE DIALOG"
      cmd = json.dumps(self.complete_msg({"type":"CERT"}))
      resp = self.tranceive(cmd)

if __name__ == '__main__':
    if len(sys.argv) > 1:
      exe = sys.argv[1]
      # remove argument so that unittest.main() would work as expected
      sys.argv = [sys.argv[0]]
    else:
      print "usage: stateless-test.py <path to executable>"
      sys.exit(1)
    # run tests
    unittest.main()
