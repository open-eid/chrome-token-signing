# Chrome Native Client plugin

**Now available from [Chrome Web Store](https://chrome.google.com/webstore/detail/ckjefchnfjhjfedoccjbhjpbncimppeg)**

**Info: Firefox version 50 supports Chrome extensions and is implemented in version 1.0.4.**

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building
[![Build Status](https://travis-ci.org/open-eid/chrome-token-signing.svg?branch=master)](https://travis-ci.org/open-eid/chrome-token-signing)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/open-eid/chrome-token-signing?branch=master&svg=true)](https://ci.appveyor.com/project/open-eid/chrome-token-signing)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/2449/badge.svg)](https://scan.coverity.com/projects/2449)

1. Install dependencies

   1.1 Ubuntu

        sudo apt-get install qtbase5-dev libssl-dev libpcsclite-dev qt5-default

   1.2 OpenSUSE

     * Apply this patch first: https://gist.github.com/hsanjuan/64a1484c5aa0a6cba80d6459cf00cc70

            sudo zypper in libQt5Core-devel libQt5Widgets-devel libQt5Gui-devel libQt5Network-devel libqt5-qtbase-common-devel`

   1.2 Windows

     * [Visual Studio Community 2013/2015/2017](https://www.visualstudio.com/downloads/)

   1.3 OSX

     * [XCode](https://itunes.apple.com/en/app/xcode/id497799835?mt=12)

2. Fetch the source

        git clone --recursive https://github.com/open-eid/chrome-token-signing
        cd chrome-token-signing

3. Build

        make

4. Install

    1.1 Linux

        cd host-linux
        sudo make install


## Support
Official builds are provided through official distribution point [installer.id.ee](https://installer.id.ee). If you want support, you need to be using official builds.

Source code is provided on "as is" terms with no warranty (see license for more information). Do not file Github issues with generic support requests.
Contact for assistance by email abi@id.ee or http://www.id.ee
