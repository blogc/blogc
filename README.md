# blogc

[![Build Status](https://semaphoreci.com/api/v1/projects/bd67545c-8593-4a37-ba94-ef1187a6d58d/402577/badge.svg)](https://semaphoreci.com/blogc/blogc)

A blog compiler.


## Quickstart

Clone the [Git repository](https://github.com/blogc/blogc) or grab the [latest release](https://github.com/blogc/blogc/releases) and extract it.

Inside the source directory, run the following commands:

    $ ./autogen.sh  # if installing from git
    $ ./configure
    $ make
    # make install

To create your first blog, please clone our example repository and adapt it to your needs:

    $ git clone https://github.com/blogc/blogc-example my-blog
    $ cd my-blog
    $ rm -rf .git
    $ git init
    $ git commit -am 'initial commit'

At this point you'll have an empty blog, that can be customized to suit your needs. You'll want to look at the 'post/' directory and edit your first post. Each new post, template or asset must be added to the Makefile. Please read it carefully.

If some unexpected error happened, please [file an issue](https://github.com/blogc/blogc/issues/new).

-- Rafael G. Martins <rafael@rafaelmartins.eng.br>
