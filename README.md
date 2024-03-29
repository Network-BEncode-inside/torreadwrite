# torreadwrite

Utilities `torread` for decoding and `torwrite` for encoding `.torrent` files in `bencode`.

| VL-LUG |   |
| --- | --- |
| XMPP: | <xmpp:linuxdv@conference.jabber.ru> |
| URL: | http://www.linuxdv.org/wiki/projects/torrents_editor |
| SRC: | http://linuxdv.ru/forum/download/file.php?id=28 |


### BUILD

```shell
$ make
```

### INSTALL

```shell
$ sudo cp torread torwrite /usr/bin
$ sudo cp torreadwrite.1.gz /usr/share/man/man1
$ sudo ln -s /usr/share/man/man1/torreadwrite.1.gz /usr/share/man/man1/torread.1.gz
$ sudo ln -s /usr/share/man/man1/torreadwrite.1.gz /usr/share/man/man1/torwrite.1.gz
```

### SAMPLE

```shell
$ torread samplefile.torrent samplefile.torrent.txt
$ nano samplefile.torrent.txt
$ torwrite samplefile.torrent.txt samplefile.mod.torrent
```

### STRUCTURE

#### Content torrent file

Content torrent file is encoded as described above. Himself torrent file is bencoded dictionary with the following keys:

* **info**: a dictionary describing the files in the torrent. There are two forms of the dictionary, the first to the torrent that contains only one file, and the second - for multi file torrent.
* **announce**: string with a URL of the tracker.
* **announce-list**: (optional) list of lists, each of which contains a string with a URL of the tracker.
* **creation date**: (optional) integer - the creation of a torrent in seconds era UNIX (number of seconds since 00:00:00 01/01/1970).
* **comment**: (optional) string with an arbitrary comment.
* **created by**: (optional) string with the name and version of the created torrent file program.
* **encoding**: (optional) any string of unknown purpose.

In beztrekernom torrent announce and no announce-list, but instead there is an element nodes, which is a list of lists, each of which contains a line with the address and the number of nodes - the port number. Something like nodes = `[["<host>", <port>], [«<host>», <port>], ...]`. The simplest option - nodes = `[["127.0.0.1", 6881]]`.

#### Dictionary info

_The parameters are the same for single-file and multi-file torrents_.

* **piece length**: number of bytes in a piece, usually a power of two.
* **pieces**: a string of united 20-byte SHA1 hash pieces.
* **private**: (optional) number. If it is one, the customer to search for peers to be used only tracker (s) specified in the torrent file. If this number is zero, the customer can add peers any methods: manually, or via DHT and Peer Exchange t. D.

_For single file torrents_:

* **name**: string filename.
* **length**: the number of bytes in the file.
* **md5sum**: (optional) string MD5 sums file. Fucking useless.

_For multi file torrent_:

* **name**: string with the name of the directory where all files will be placed.
* **files**: a list of dictionaries, one for each file. Each dictionary contains the following keys:
* **length**: the number of bytes in the file.
* **md5sum**: (optional) string MD5 sums file. Fuck no one needs, as in the previous case.
* **path**: a list of one or more lines, representing together a file path. The last line - the name of the file, the previous - the sequence of nested directories. For example, the path `dir1/dir2/file.ext` will be presented in a list of three lines: `dir1`, `dir2`, `file.ext`.
