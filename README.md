# Parallel brute force

This code is made for breaking strings from a data-set into passwords that were encrypted with SHA-256. It uses a brute-force method that tests all the possible outcomes for a SHA-256 encrypted password of a certain given length and compares it with the string to be decrypted.

**What is SHA-256?**

SHA-256 is one of the most secure hashing functions on the market. SHA-256 is used in some of the most popular authentication and encryption protocols, including SSL, TLS, IPsec, SSH, and PGP. In Unix and Linux, SHA-256 is used for secure password hashing. Cryptocurrencies such as Bitcoin use SHA-256 for verifying transactions.

**What does our code do?**

We started from [this code](https://rosettacode.org/wiki/Parallel_brute_force#C) that only worked on some hard-coded strings of a static length and printed their decryption. Now our code can identify passwords of any length under a given number. Because the SHA-256 brute-force approach takes exponential time more as you have longer passwords, we limited our code only for a password length of 5. Furthermore, the strings to be decrypted are kept in a buffer that can be used to read 64 bits SHA-256 ciphers from the address of a file descriptor (like the .txt file we used or even a TCP socket).
