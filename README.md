# otp
Portfolio project for my CS344 Operating Systems class at Oregon State University.

## Specifications
This program will encrypt and decrypt plaintext into ciphertext, using a key and modulo 27 operations. The 27 characters are the 26 capital letters, and the space character. All 27 characters will be encrypted and decrypted.

There are a total of five small programs in C. 
* Two of these will function as servers, and will be accessed using network sockets.
* Two will be clients, each one of these will use one of the servers to perform work.
* One last program is a standalone utility.

Programs will use the API for network IPC that was discussed in the class (`socket`, `connect`, `bind`, `listen`, & `accept` to establish connections; `send`, `recv` to send and receive sequences of bytes) for the purposes of encryption and decryption by the appropriate servers.

### enc_server / dec_server
This program is the server and will run in the background as a daemon.

- Its function is to perform the actual encoding or decoding.
- This program will listen on a particular port/socket, assigned when it is first ran.
- Upon execution, `server` must output an error if it cannot be run due to a network error, such as the ports being unavailable.
- When a connection is made, `server` must call `accept` to generate the socket used for actual communication, and then use a separate process to handle the rest of the servicing for this client connection (see below), which will occur on the newly accepted socket.
- This child process of `server` must first check to make sure it is communicating with its respective `server`.
- After verifying the connection, this child receives plaintext and a key from `client` via the connected socket.
- The `server` child will then write back the ciphertext to the `client` process that it is connected to via the same connected socket.
- Note that the key passed in must be at least as big as the plaintext.

This version of `server` must support up to five concurrent socket connections running at the same time; this is different than the number of client connection requests that could queue up on your listening socket (which is specified in the second parameter of the `listen` call). Again, only in the child server process will the actual encryption take place, and the ciphertext be written back: the original server daemon process continues listening for new connections, not encrypting data.

### enc_client / dec_client
This program connects to `server`, and asks it to perform a one-time pad style encryption as detailed above. By itself, `client` doesn’t do the encryption - `server` does. The syntax of `client` is as follows:

```
client plaintext key port
```

In this syntax, `plaintext` is the name of a file in the current directory that contains the plaintext you wish to encrypt. Similarly, `key` contains the encryption key you wish to use to encrypt the text. Finally, `port` is the port that `client` should attempt to connect to `client` on. When `client` receives the ciphertext back from `server`, it should output it to `stdout`. Thus, `client` can be launched in any of the following methods, and should send its output appropriately:

```
$ enc_client myplaintext mykey 57171
$ enc_client myplaintext mykey 57171 > myciphertext
$ enc_client myplaintext mykey 57171 > myciphertext &

```

If `client` receives key or plaintext files with ANY bad characters in them, or the key file is shorter than the plaintext, then it should terminate, send appropriate error text to stderr, and set the exit value to 1.

Again, any and all error text must be output to `stderr` (not into the plaintext or ciphertext files).

### keygen
This program creates a key file of specified length. The characters in the file generated will be any of the 27 allowed characters, generated using the standard Unix randomization methods. Do not create spaces every five characters, as has been historically done. Note that you specifically do not have to do any fancy random number generation: we’re not looking for cryptographically secure random number generation. [rand() Links to an external site.](https://man7.org/linux/man-pages/man3/rand.3.html) is just fine. The last character keygen outputs should be a newline. Any error text must be output to stderr.

The syntax for `keygen` is as follows:

```
keygen keylength
```

where `keylength` is the length of the key file in characters. `keygen` outputs to `stdout`.

Here is an example run, which creates a key of 256 characters and redirects `stdout` a file called `mykey` (note that `mykey` is 257 characters long because of the newline):

```
$ keygen 256 > mykey
```
