bf-llvm
=======

What?
-----

A tiny JIT-compiler for [Brainfuck](https://en.wikipedia.org/wiki/Brainfuck).

Why?
----

To play with [C++11](https://en.wikipedia.org/wiki/C%2B%2B11) and
[LLVM](http://llvm.org).

How?
----

I have found that simply and reliably building things with C++11 and LLVM is,
with the best will in the world, a freakin' nightmare. But hopefully, `build.sh`
should take care of it for you. You have two options:

1. Be on OSX with [Homebrew](http://brew.sh) installed, and run
    
        $ ./build.sh --osx

  This will `brew install llvm33`, which seems to compile LLVM, and therefore
  takes a few minutes. You probably don't want to be on battery for that. I have
  only tested this on OSX 10.9.

2. Be on something else with [Vagrant](http://vagrantup.com) installed, and run

        $ ./build.sh --vagrant-host

  This will build and start a Ubuntu 12.04 VM for you and then download and
  install LLVM and Clang. It will download about 400MB of stuff. You probably
  don't want to be on a mobile internet connection for that. I have also only
  tested this on OSX 10.9.

You should see `Hello World!` which is what

    ++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.
    +++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.

is supposed to print.

Issues
------

* I think casting to `thunk` and calling directly is no longer the Right Way To
  Do It. (I wrote this ages ago and only just made it compile with LLVM 3.3.)

* I have probably cargo-culted a load of other unnecessary LLVM stuff.

* `llvm-config --libs all` is overkill.

* No error checking.

* BF program is hard-coded.

Credit
------

The Vagrant set up is "strongly inspired" by the one in mozilla/playdoh.
