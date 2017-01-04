# blogc

[![Build Status](https://travis-ci.org/blogc/blogc.svg?branch=master)](https://travis-ci.org/blogc/blogc)

A blog compiler.


## Quickstart

Clone the [Git repository](https://github.com/blogc/blogc) or grab the [latest release](https://github.com/blogc/blogc/releases) and extract it.

If installing from Git repository, [ronn](https://github.com/rtomayko/ronn) and [GNU Autotools](http://www.gnu.org/software/automake/manual/html_node/Autotools-Introduction.html) must be installed in the machine. Release tarballs does not have these dependencies and are the recommended way to install.

Inside the source directory, run the following commands:

    $ ./autogen.sh  # if installing from git
    $ ./configure [--enable-git-receiver] [--enable-make] [--enable-runserver]
    $ make
    # make install

The `./configure` options listed above will enable building of helper tools. To learn more about these tools, please read the man pages.

To create your first blog, please clone our example repository and adapt it to your needs:

    $ git clone https://github.com/blogc/blogc-example my-blog
    $ cd my-blog
    $ rm -rf .git
    $ git init
    $ git commit -am 'initial commit'

At this point you'll have an empty blog, that can be customized to suit your needs. You'll want to look at the `content/post/` directory and edit your first post. Each new post, template or asset must be added to the `Makefile`. Please read it carefully.

If some unexpected error happened, please [file an issue](https://github.com/blogc/blogc/issues/new).

-- Rafael G. Martins <rafael@rafaelmartins.eng.br>
