# Chrome Native Client plugin

**Now available from [Chrome Web Store](https://chrome.google.com/webstore/detail/ckjefchnfjhjfedoccjbhjpbncimppeg)**

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building
[![Build Status](https://travis-ci.org/open-eid/chrome-token-signing.svg?branch=master)](https://travis-ci.org/open-eid/chrome-token-signing)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/2449/badge.svg)](https://scan.coverity.com/projects/2449)

1. Install dependencies

   1.1 Ubuntu

        sudo apt-get install qtbase5-dev libssl-dev

   1.2 Windows

     * [Visual Studio Express 2013 for Windows Desktop](http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx)
     * [http://qt-project.org](http://qt-project.org)

   1.3 OSX

     * [XCode](https://itunes.apple.com/en/app/xcode/id497799835?mt=12)
     * [http://qt-project.org](http://qt-project.org)

2. Fetch the source

        git clone --recursive https://github.com/open-eid/chrome-token-signing
        cd chrome-token-signing

3. Build

        make

## Support
Official builds are provided through official distribution point [installer.id.ee](https://installer.id.ee). If you want support, you need to be using official builds.

Source code is provided on "as is" terms with no warranty (see license for more information). Do not file Github issues with generic support requests.
Contact for assistance by email abi@id.ee or http://www.id.ee
