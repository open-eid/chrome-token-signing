# Chrome Native Client plugin

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building
Currently only supports Linux (Debian derivatives)

1. Install dependencies

    sudo apt-get install libgtkmm-3.0-dev libssl-dev

2. Fetch the source

    git clone --recursive https://github.com/open-eid/chrome-token-signing
    cd chrome-token-signing
    make

3. Test!

    make test

## Support
Official builds are provided through official distribution point [installer.id.ee](https://installer.id.ee). If you want support, you need to be using official builds.

Source code is provided on "as is" terms with no warranty (see license for more information). Do not file Github issues with generic support requests.
