#!/usr/bin/python
# -*- coding: utf-8 -*-

import json
import subprocess
import struct
import sys
import unittest
import uuid
import re

# The protocol datagram is described here:
# https://developer.chrome.com/extensions/nativeMessaging#native-messaging-host-protocol
#
# The protocol itself is described here:
# https://github.com/open-eid/chrome-token-signing/wiki/NativeMessagingAPI


def instruct(msg):
    raw_input('>>>>>> %s\n[press ENTER to continue]' % msg)

def get_exe():
    if sys.platform == 'darwin':
        return "host-osx/build/Release/chrome-token-signing.app/Contents/MacOS/chrome-token-signing"
    elif sys.platform == "linux2":
        return "host-linux/out/chrome-token-signing"
    else:
        print("Unsupported platform: %s" % sys.platform)
        sys.exit(1)

class TestLongrunningHost(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        should_close_fds = sys.platform.startswith('win32') == False
        cls.p = subprocess.Popen(get_exe(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=should_close_fds, stderr=None)
        print('Running native component on PID %d' % cls.p.pid)

    @classmethod
    def tearDownClass(cls):
        cls.p.terminate()
        cls.p.wait()

    def transceive(self, msg):
        print('SEND: %s' % msg)
        self.p.stdin.write(struct.pack('=I', len(msg)))
        self.p.stdin.write(msg)
        response_length = struct.unpack('=I', self.p.stdout.read(4))[0]
        response = str(self.p.stdout.read(response_length))
        response_print = json.dumps(json.loads(response))
        print('RECV: %s' % response_print)
        return json.loads(response)

    def complete_msg(self, msg):
        msg['nonce'] = str(uuid.uuid4())
        msg['lang'] = 'en'
        msg['origin'] = 'https://example.com/test'
        return msg

    def test_nonce_echo(self):
        cmd = self.complete_msg({'type': 'VERSION'})
        original_nonce = cmd['nonce']
        resp = self.transceive(json.dumps(cmd))
        self.assertEqual(resp['nonce'], original_nonce)

    def test_version(self):
        cmd = json.dumps(self.complete_msg({'type': 'VERSION'}))
        resp = self.transceive(cmd)
        self.assertTrue(resp['version'] == 'LOCAL_BUILD' or re.compile("^\d\.\d+\.\d{1,3}$").match(resp['version']))

    def test_version_file(self):
        msg = self.complete_msg({'type': 'VERSION'})
        msg['origin'] = 'file:///tmp/some.html'
        cmd = json.dumps(msg)
        resp = self.transceive(cmd)
        self.assertEquals(resp['result'], 'ok')
        self.assertTrue(resp['version'] == 'LOCAL_BUILD' or re.compile("^\d\.\d+\.\d{1,3}$").match(resp['version']))

    def test_get_certificate_cancel(self):
        instruct('Insert card and press CANCEL in dialog')
        cmd = json.dumps(self.complete_msg({'type': 'CERT'}))
        resp = self.transceive(cmd)
        self.assertEquals(resp['result'], 'user_cancel')

    def test_get_certificate_error(self):
        instruct('Insert card and try to REMOVE it while reading the certificate')
        cmd = json.dumps(self.complete_msg({'type': 'CERT'}))
        resp = self.transceive(cmd)
        self.assertTrue(resp['result'] == 'technical_error' or resp['result'] == 'no_certificates')

    def test_get_certificate_ok(self):
        instruct('Insert card and select certificate')
        cmd = json.dumps(self.complete_msg({'type': 'CERT'}))
        resp = self.transceive(cmd)
        self.assertEquals(resp['result'], 'ok')

    def test_get_certificate_none(self):
        instruct('Remove card from reader')
        cmd = json.dumps(self.complete_msg({'type': 'CERT'}))
        resp = self.transceive(cmd)
        self.assertEquals(resp['result'], 'no_certificates')

    def test_get_certificate_no_reader(self):
        instruct('Remove reader')
        cmd = json.dumps(self.complete_msg({'type': 'CERT'}))
        resp = self.transceive(cmd)
        self.assertEquals(resp['result'], 'no_certificates')

    def test_get_certificate_and_sign_ok(self):
        instruct('Insert card, select certificate and sign successfully')
        cmd = json.dumps(self.complete_msg({'type': 'CERT'}))
        resp = self.transceive(cmd)
        self.assertEquals(resp['result'], 'ok')
        self.assertTrue(len(resp['cert']) > 100)
        cmd2 = json.dumps(self.complete_msg({'type': 'SIGN', 'hash': '0102030405060708090a0b0c0d0e0f0102030405', 'cert': resp['cert']}))
        resp2 = self.transceive(cmd2)
        self.assertEquals(resp2['result'], 'ok')
        self.assertTrue('signature' in resp2)

    def test_get_certificate_and_sign_badhash(self):
        instruct('Insert card, select certificate')
        cmd = json.dumps(self.complete_msg({'type': 'CERT'}))
        resp = self.transceive(cmd)
        self.assertEquals(resp['result'], 'ok')
        self.assertTrue(len(resp['cert']) > 100)
        cmd2 = json.dumps(self.complete_msg({'type': 'SIGN', 'hash': '0102030405060708090a0b0c0d0e0f010203', 'cert': resp['cert']}))
        resp2 = self.transceive(cmd2)
        self.assertEquals(resp2['result'], 'invalid_argument')


if __name__ == '__main__':
    unittest.main()
