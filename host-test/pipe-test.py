import json
import subprocess
import struct
import sys
import unittest

# The protocol datagram is described here:
# https://developer.chrome.com/extensions/nativeMessaging#native-messaging-host-protocol

def get_exe():
    if sys.platform == 'darwin':
        return "host-osx/build/Release/chrome-token-signing.app/Contents/MacOS/chrome-token-signing"
    elif sys.platform == "linux2":
        return "host-linux/out/chrome-token-signing"
    elif sys.platform == 'win32':
        return "host-windows\\Debug\\host-windows.exe"
    else:
        print("Unsupported platform: %s" % sys.platform)
        sys.exit(1)

class TestHostPipe(unittest.TestCase):

  def get_response(self):
      response_length = struct.unpack("=I", self.p.stdout.read(4))[0]
      response = str(self.p.stdout.read(response_length))
      # make it into "oneline" json before printing
      response_print = json.dumps(json.loads(response))
      print ("RECV: %s" % response_print)
      return json.loads(response)

  def transceive(self, msg):
      # send like described in 
      print ("SEND: %s" % msg)
      self.p.stdin.write(struct.pack("=I", len(msg)))
      self.p.stdin.write(msg)
      # now read the input
      return self.get_response()

  def setUp(self):
      should_close_fds = sys.platform.startswith('win32') == False;
      self.p = subprocess.Popen(get_exe(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=should_close_fds, stderr=None)
      print ("Running native component on PID %d" % self.p.pid)

  def tearDown(self):
      self.p.terminate()
      self.p.wait()

  def test_random_string(self):
      cmd = "BLAH"
      resp = self.transceive(cmd)
      self.assertEquals(resp["result"], "invalid_argument")

  def test_plain_string(self):
      self.p.stdin.write("Hello World!")
      resp = self.get_response()
      self.assertEquals(resp["result"], "invalid_argument")

  def test_utopic_length(self):
      # write big bumber and little data
      self.p.stdin.write(struct.pack("=I", 0xFFFFFFFF))
      self.p.stdin.write("Hello World!")
      resp = self.get_response()
      self.assertEquals(resp["result"], "invalid_argument")
  
if __name__ == '__main__':
    # run tests
    unittest.main()
