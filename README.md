#Riakc PHP Extension

This is a work in progress Riak protobuf PHP extension. It wraps the [riak-cxx-client](http://github.com/basho/riak-cxx-client "riak-cxx-client") and will hopefully implement most, if not all features of the riak-cxx-client.

##Installation

Make sure the riak-cxx-client is compiled and installed as the extension needs the library and header files.


    phpize
    ./configure --with-riakc[=DIR]
    make
    sudo make install


Then add the extension to your php.ini file.

    extension=riak.so


##Example

```php
$c = new RiakClient("127.0.0.1", "8087");

$o = new RiakObject($c, "tbucket", "tkey");

$o->contentType("application/json");

$o->setValue("{\"name\":\"mike\"}");

$o->store(); // bool(true)

$o2 = $c->get("tbucket", "tkey");

$o2->setValue("{\"name\":\"joe\"}");

$o2->store(); // bool(true)

echo $o2->getValue(); // "{\"name\":\"joe\"}"

$c->del("tbucket", "tkey"); // bool(true)

$c->get("tbucket", "tkey"); // bool(false)
  
```

##TODO
This is certainly a work in progress. There are a host of things to do:

* Add class properties for things besides content-type
* Make sure all possible configuration parameters are available
* Add functions besides get/store/delete
* Fix memory leaks
* Sibling resolution
* Setting properties on buckets
* Do proper conversions of char pointers to string, those warnings are annoying
* Exception handling