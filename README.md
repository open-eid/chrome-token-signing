# Chrome Native Client plugin

![European Regional Development Fund](https://github.com/e-gov/RIHA-Frontend/raw/master/logo/EU/EU.png "European Regional Development Fund - DO NOT REMOVE THIS IMAGE BEFORE 05.03.2020")

**Now available from [Chrome Web Store](https://chrome.google.com/webstore/detail/ckjefchnfjhjfedoccjbhjpbncimppeg)**
**Now available from [Windows Store](https://microsoftedge.microsoft.com/addons/detail/fofaekogmodbjplbmlbmjiglndceaajh)**

**Info: Firefox version 50 supports Chrome extensions and is implemented in version 1.0.4.**

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building
[![Build Status](https://travis-ci.com/open-eid/chrome-token-signing.svg?branch=master)](https://travis-ci.com/open-eid/chrome-token-signing)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/open-eid/chrome-token-signing?branch=master&svg=true)](https://ci.appveyor.com/project/open-eid/chrome-token-signing)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/2449/badge.svg)](https://scan.coverity.com/projects/2449)

1. Install dependencies

   1.0 blackPanther OS

        installing qtbase5-common-devel libopenssl-devel libpcsclite-devel libqt5core-devel libqt5network-devel libqt5widgets-devel
         (or binary package from official repository : installing chrome-token-signing)

   1.1 Ubuntu

        sudo apt-get install qtbase5-dev libssl-dev libpcsclite-dev qt5-default

   1.2 Windows

     * [Visual Studio Community 2015/2017/2019](https://www.visualstudio.com/downloads/)

   1.3 OSX

     * [XCode](https://itunes.apple.com/en/app/xcode/id497799835?mt=12)

        
2. Fetch the source

        git clone --recursive https://github.com/open-eid/chrome-token-signing
        cd chrome-token-signing

3. Build locally

    3.1 Ubuntu

        cd host-linux
        make 
        make debian

    3.2 Windows

        nmake
        nmake pkg-unsigned

    3.3 OSX

        cd host-osx
        make
        make pkg

## Support
Official builds are provided through official distribution point [id.ee](https://www.id.ee/en/article/install-id-software/). If you want support, you need to be using official builds.

Source code is provided on "as is" terms with no warranty (see license for more information). Do not file Github issues with generic support requests.
Contact our support via [www.id.ee](http://www.id.ee) for assistance.
