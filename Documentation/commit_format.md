refact skciphter.

Description:
At present, i found there's no need shared same extract class between
Skcipher and Akcipher, so i decide to refact relevent code.

Major changes:
1. change CipherAlgo to SkcipherAlgo
2. change CipherMode to SkcipherMode
