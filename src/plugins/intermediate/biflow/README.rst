Biflow (intermediate plugin)
===================================

The plugin is connecting related uniflow records into biflow records according to RFC 5103 standard. Structured data
types (i.e. subTemplateLists, and others) are not yet supported by the plugin. Flow records are paired regardless of the ODID.

Example configuration
---------------------

.. code-block:: xml

    <intermediate>
        <name>Biflow</name>
        <plugin>biflow</plugin>
        <params>
    	<timeoutCache>5</timeoutCache>
    	<timeoutMessage>0</timeoutMessage>
        <ignoreMissingReverse>false</ignoreMissingReverse>
    	<pairMissingPorts>true</pairMissingPorts>
        <odid>5</odid>
        </params>
    </intermediate>

Parameters
----------

:``timeoutCache``:
     A minimal number of seconds that the record can be in the cache. The parameter should be set such that a record can
     be freely removed from the cache after expiry because it is not expected that reversed flow will arrive.
     This parameter is optional. The default value is 5 seconds.

:``timeoutMessage``:
    A maximal number of seconds for a message between initializing and sending to the next module. This parameter is
    convenient in low-traffic networks. This is an optional parameter with a default value of 10 seconds.

:``ignoreMissingReverse``:
   In case the reverse definition is missing for elements, which needs to be reversed, elements can be dropped
   (option true) or sent as forward elements (option false). The parameter is optional, with a default value of false.

:``pairMissingPorts``:
    If ports are missing in a record, such records are not paired (option false) or are paired according to IP addresses
    and protocol (option true). It is an optional parameter, the default being false.

:``odid``:
   Records are paired regardless of origin, but output ODID can be set with this parameter. Compulsory parameter.

Notes
-----

Enterprise-specific Information Elements are also supported. However, if aa reverse element does not occur in the output,
even if it should make sure that the particular Information Element is
defined among other definitions provided by `libfds <https://github.com/CESNET/libfds/>`_ library.
Mainly in case of Enterprise-Specific Information Elements, there is a chance that the
definitions are missing. See the documentation of the library, for help to easily add extra
definitions in few steps.
