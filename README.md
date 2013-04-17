pip3line
========

Raw bytes manipulation utility, able to apply well known and less well known transformations.

> Released as open source by NCC Group Plc - http://www.nccgroup.com/

> Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

> https://github.com/nccgroup/pip3line

> Released under AGPL see LICENSE for more information

[Windows binaries]

https://code.google.com/p/pip3line/downloads/list
*Read the download instructions carefully*

[Documentation]

Native plugin API
https://github.com/nccgroup/pip3line/wiki/Native-plugin-API

Python plugins
https://github.com/nccgroup/pip3line/wiki/Python-plugins

Building process
https://github.com/nccgroup/pip3line/wiki/Build-Process

Changelog
https://github.com/nccgroup/pip3line/wiki/Changelog

Shortcuts list
https://github.com/nccgroup/pip3line/wiki/Shortcuts-list

[Description]

Pip3line is a tool for manipulating raw bytes blocks, and isable to apply diverses transformations to them. 

Its main usefullness is for pentesting and reverse-engineering / binary analysis purposes.

Current transformations list include classic decoders such as Base64/32/hex to simple cryptographic ciphers, and includes common hashes algorithms as well as obfuscation techniques.

Easy to use, but still offering some tweaking for most transformations, it also has the ability to save/restore a configured transformation chain for future used.

The tool is also able to apply a transformation chain _en masse_ to either a file, or data blocks received from a TCP scoket, a Named Pipe (Windows) or a UNIX Socket (UNIX/Linux) via the *mass processing* utility

It is also extendable for you to create your own transformations by using the C++/Qt plugin API.

Alternatively you can just use the powerful Python 2.7 & 3 plugins to load your own Python scripts.

[Capabilities]

Transformations currently implemented :

* Base32 (RFC 4648, Crockford, RFC 2938 a.k.a base32hex)
* Base64 (RFC 4648, ".Net" special version for *Resource.axd, Urlsafe)
* Binary encoding
* Bytes to Integer
* Char encoding (Unicode, iso ...)
* Cisco secret 7 decryption/encryption
* Cut
* FIX protocol parser (v4.4)
* Hexadecimal
* Hieroglyphy For JavaScript obfuscation
* Html
* Int to Timestamp (Epoch)
* Int64 to Timestamp (Microsoft)
* IPv4 translator between number and string representation of an IP
* Netmask calculator for IPv4 and IPv6
* Md4/Md5/Sha1 (built-in)
* NTLMSSP Messages parser
* Padding (Zero, ANSI X.923, ISO 10126, PKCS7, custom single char)
* Random Case
* Regular Expression (match&extract, match&replace)
* Reverse
* ROTx (ROT13, ROT5, ROT47)
* Signed Short to Char decoder
* Split
* Substitution crypto algorithm
* Oracle/MySql/MSSql/Postgres/Javascript concatenated string
* Url Encode
* Xor
* XmlQuery (XPATH)
* Zlib compression

Current additional Plugins:

* OpenSSL hashes (Md4/Md5/Sha*/Whirlpool/MDC2/RIPEMD160)
* Python 2.7 & 3.x to run Python 2.7 and 3.x scripts

