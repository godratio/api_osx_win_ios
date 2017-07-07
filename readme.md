# API_OSX_WIN

These are used/developed for a game in production.
The purpose and target for them is similar to the stb libs.
If your looking for a more complete and well tested set of functionality
you might want to look <link>there<link> instead.

The main difference is here the focus is on using platform specific api's
to develop platform specific api's that just work and do not allocate memory
(unless the platform is allocating behind our backs which I may or may not be 
able to do anything about.)

## Negatives
1. Still using some std libs.
2. Not tested throughouly.
3. api not stable.
4. Way to integrate into projects not decided.
5. Have not yet allowed to take in raw buffers for full c compatibility and easier integration into any projects.
    5a. In other words have to use the data types provided at the moment.

Most of these will evenutally be fixed any help is appreciated.

## Restrictions placed here

1. Memory allocations are explicit.
2. Single memory startup and try to maximize allocated memory resuse.
ie not calling memory allocation functions as little as possible.
3. Simple as possible.
4. Single file includes if possible
5. Minimize the number of things user code has to do to get it to "just work"
6. ...


The following was Ripped from stb libs.  Same exact reasoning here.

#### Why single-file headers?

Windows doesn't have standard directories where libraries
live. That makes deploying libraries in Windows a lot more
painful than open source developers on Unix-derivates generally
realize. (It also makes library dependencies a lot worse in Windows.)

There's also a common problem in Windows where a library was built
against a different version of the runtime library, which causes
link conflicts and confusion. Shipping the libs as headers means
you normally just compile them straight into your project without
making libraries, thus sidestepping that problem.

Making them a single file makes it very easy to just
drop them into a project that needs them. (Of course you can
still put them in a proper shared library tree if you want.)

Why not two files, one a header and one an implementation?
The difference between 10 files and 9 files is not a big deal,
but the difference between 2 files and 1 file is a big deal.
You don't need to zip or tar the files up, you don't have to
remember to attach *two* files, etc.


#### What's the license?

These libraries are in the public domain. You can do anything you
want with them. You have no legal obligation
to do anything else, although I appreciate attribution.

