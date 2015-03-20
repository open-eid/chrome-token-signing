#!/usr/bin/python
# -*- coding: utf-8 -*-
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

class TestLongrunningHost(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        should_close_fds = sys.platform.startswith('win32') == False
        cls.p = subprocess.Popen(testconf.get_exe(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=should_close_fds, stderr=None)
        print('Running native component on PID %d' % cls.p.pid)

    @classmethod
    def tearDownClass(cls):
        cls.p.terminate()
        cls.p.wait()

    def transceive(self, msg):
        print('SEND: %s' % msg)
        self.p.stdin.write(struct.pack('=I', len(msg)))
        self.p.stdin.write(msg)
        print("Waiting for response...")
        response_length = struct.unpack('=I', self.p.stdout.read(4))[0]
        print("Response length: %i" % response_length)
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
        self.assertTrue(resp['version'] == 'LOCAL_BUILD' or re.compile("^\d\.\d+\.\d+\.\d{1,3}$").match(resp['version']))

    def test_version_file(self):
        msg = self.complete_msg({'type': 'VERSION'})
        msg['origin'] = 'file:///tmp/some.html'
        cmd = json.dumps(msg)
        resp = self.transceive(cmd)
        self.assertEquals(resp['result'], 'ok')
        self.assertTrue(resp['version'] == 'LOCAL_BUILD' or re.compile("^\d\.\d+\.\d+\.\d{1,3}$").match(resp['version']))

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

    def test_get_certificate_and_sign_with_wrong_certificate(self):
        wrongCert = "308204763082035EA003020102021006530D232DBC49745020DA8974CC6D4E300D06092A864886F70D0101050500306C310B300906035504061302454531223020060355040A0C19415320536572746966697473656572696D69736B65736B7573311F301D06035504030C1654455354206F66204553544549442D534B20323031313118301606092A864886F70D0109011609706B6940736B2E6565301E170D3132303830373039303631365A170D3137303333313230353935395A3081A4310B300906035504061302454531193017060355040A0C104553544549442028444947492D494429311A3018060355040B0C116469676974616C207369676E61747572653124302206035504030C1BC5BD41494B4F56534B492C49474F522C33373130313031303032313113301106035504040C0AC5BD41494B4F56534B49310D300B060355042A0C0449474F52311430120603550405130B33373130313031303032313081A0300D06092A864886F70D010101050003818E0030818A02818100BD515573A0CCAB4EBC5D8CFA76EA2BFA7EBA48FE9F44097086653774F1DD3293796D6E406B976CC3D221D1CDFB32E1AF565D8E639A23A46C2FCEBCF4ABB3224A44DDBD109DE11F7B40845579E1FC47E5EC377AD76C72F2DFC935CBE3C9F6A4725234C7A0747A4BD8E34BABF675613E06270AE1A883E101DD8BFCC7DA66540F61020471667FD9A382015C3082015830090603551D1304023000300E0603551D0F0101FF0404030206403081990603551D2004819130818E30818B060A2B06010401CE1F030201307D305806082B06010505070202304C1E4A00410069006E0075006C0074002000740065007300740069006D006900730065006B0073002E0020004F006E006C007900200066006F0072002000740065007300740069006E0067002E302106082B060105050702011615687474703A2F2F7777772E736B2E65652F6370732F301D0603551D0E0416041471F6E3C9B1341211A69F434C0E9AD930C3DE9DC8301806082B06010505070103040C300A3008060604008E460101301F0603551D2304183016801441B6FEC5B1B1B453138CFAFA62D0346D6D22340A30450603551D1F043E303C303AA038A0368634687474703A2F2F7777772E736B2E65652F7265706F7369746F72792F63726C732F746573745F657374656964323031312E63726C300D06092A864886F70D010105050003820101005DF9FACDE817A9D21BF969B7A9E81A5A716C761861FBF41A938C2ED74D72BDE63A69FA946BE662D6BE3AD08879C637CBCE3BF99F9306113A7A79D34E19C6FD57B2B0B16167F5293500CCDEA186CB58516707292B7DC72F72B5315236A7D6E5EAA2E0B1C01C276545ACAED76DFDF8F7C18EBC63D567014E831A93C4C86C1063E7FEBBBAE9C404E9D9FD3AB54ABC0F0D579E32A2EDA74F1DE219102D27D4412B534E19BF68B10483FB66620B1A44C6F9D2C1B1DBF1B9250D06ADBEFA0EFCD8393E5C0E7FD60E8A86A3B929E274C4AFCF6D9E0A8B5BA80D0C7E7E3EAA6EB592B39F9AA7181B07992E73A1E07E6F3ECE597E76CD4546690AA7E25FA0A853892B33CD"
        instruct('Insert card, select certificate')
        cmd = json.dumps(self.complete_msg({'type': 'CERT'}))
        resp = self.transceive(cmd)
        self.assertEquals(resp['result'], 'ok')
        self.assertTrue(len(resp['cert']) > 100)
        cmd2 = json.dumps(self.complete_msg({'type': 'SIGN', 'hash': '0102030405060708090a0b0c0d0e0f0102030405', 'cert': wrongCert}))
        resp2 = self.transceive(cmd2)
        self.assertEquals(resp2['result'], 'invalid_argument')


if __name__ == '__main__':
    if len(sys.argv) == 2:
        # Run a single test given in the argument
        singleTestName = sys.argv[1]
        suite = unittest.TestSuite()
        suite.addTest(TestLongrunningHost(singleTestName))
        runner = unittest.TextTestRunner()
        # Run test suite with a single test
        runner.run(suite)
    else:
        # Run all the tests
        unittest.main()
