# Chrome Native Client plugin

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building
Note: currently only supports Linux (Debian derivatives).

[![Build Status](https://travis-ci.org/open-eid/chrome-token-signing.svg?branch=master)](https://travis-ci.org/open-eid/chrome-token-signing)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/2449/badge.svg)](https://scan.coverity.com/projects/2449)

1. Install dependencies

        sudo apt-get install libgtkmm-3.0-dev libssl-dev

2. Fetch the source

        git clone --recursive https://github.com/open-eid/chrome-token-signing
        cd chrome-token-signing
3. Build

        make

4. Test

        make test

5. Install

        sudo make install

    * Then go to `chrome://extensions` in and press "Load unpacked extensions..."
    * Technical testing: https://www.openxades.org/web_sign_demo/sign.html

## Support
Official builds are provided through official distribution point [installer.id.ee](https://installer.id.ee). If you want support, you need to be using official builds.

Source code is provided on "as is" terms with no warranty (see license for more information). Do not file Github issues with generic support requests.
