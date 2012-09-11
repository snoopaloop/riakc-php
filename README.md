#Riakc PHP Extension

This is a work in progress Riak protobuf PHP extension. It wraps the [riak-cxx-client](http://github.com/basho/riak-cxx-client "riak-cxx-client") and will hopefully implement most, if not all features of the riak-cxx-client.

##Installation

Make sure the riak-cxx-client is compiled and installed as the extension needs the library and header files.

<pre><code>
  phpize
  ./configure --with-riakc[=DIR]
  make
  sudo make install
</code></pre>

Then add the extension to your php.ini file.

<pre><code>
  extension=riak.so
</code></pre>