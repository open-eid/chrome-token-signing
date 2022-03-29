# Chrome Native Client plugin

![European Regional Development Fund](https://github.com/open-eid/DigiDoc4-Client/blob/master/client/images/EL_Regionaalarengu_Fond.png "European Regional Development Fund - DO NOT REMOVE THIS IMAGE BEFORE 05.03.2020")

**NB! Please note that the active development and management of the Token Signing component has ended due to the transition to the new web authentication and signing solution (Web eID).  
We are happy to accept your proposals in the new Web eID project repository: https://github.com/web-eid.  
We won't be accepting pull requests or responding to issues for this project anymore.**

Chrome Token signing is available from [Chrome Web Store](https://chrome.google.com/webstore/detail/ckjefchnfjhjfedoccjbhjpbncimppeg) and [Microsoft Edge add-ons store](https://microsoftedge.microsoft.com/addons/detail/fofaekogmodbjplbmlbmjiglndceaajh)

**Info:** Firefox version 50 supports Chrome extensions and is implemented in version 1.0.4.

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building
[![Build Status](https://github.com/open-eid/chrome-token-signing/workflows/CI/badge.svg?branch=master)](https://github.com/open-eid/chrome-token-signing/actions)
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
