#!/usr/local/bin/python

import sys
import struct
import json

def send(msg):
    sys.stdout.write(struct.pack("=I", len(msg)))
    sys.stdout.write(msg)
    sys.stdout.flush()

def invalid_argument(msg):
    send(json.dumps({'result': 'invalid_argument', 'message': msg}))

def fail(msg):
    invalid_argument(msg)    
    sys.exit(1)

def respond(req, resp):
    resp["nonce"] = req["nonce"]
    if "result" not in resp:
        resp["result"] = "ok"
    send(json.dumps(resp))

if __name__ == "__main__":
   while True:
      try:
          # read input length
          lenbytes = sys.stdin.read(4)
          length = struct.unpack("=I", lenbytes)[0]
          if length == 0 or length > 8 *1024:
              fail("Bad length")
          # read input
          req = json.loads(sys.stdin.read(length))
          # required fields
          if not all (k in req for k in ("nonce","origin","type")):
             fail("Required fields missing")
          
          # process messages
          if req["type"] == "VERSION":
              respond(req, {"version": "0.0.34"})
          elif req["type"] == "CERT":
              if not "lang" in req:
                  invalid_argument("lang missing")
              else:
                  respond(req, {"cert": "00112233445566778899"})
          elif req["type"] == "SIGN":
              if not all (k in req for k in ("hash","cert","lang")):
                  invalid_argument("hash or cert or lang missing")
              else:
                  respond(req, {"signature": "00112233445566778899"})
          else:
              invalid_argument("unknown type")
      except Exception as e:
          print >> sys.stderr, "Exception", e
          fail("Unhandled exception")
