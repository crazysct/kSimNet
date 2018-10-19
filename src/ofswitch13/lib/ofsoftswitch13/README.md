# OpenFlow 1.3 Software Switch for ns-3

This is an [OpenFlow 1.3][ofp13] compatible user-space software switch implementation, forked from the original [CPqD OpenFlow 1.3 Software Switch][cpqdofs13] and slightly modified to proper integration with the [OFSwitch13 module][ofswitch13] for the [ns-3 Network Simulator][ns-3]. This code does not modify the original switch datapath implementation, which is currently maintained in the original repository and regularly synced to the master branch on this one. The modified `ns3lib` branch includes some callbacks, compiler directives and minor changes in struct declarations to allow integration with the OFSwitch13 module. 

# Contribute
Please, consider submitting your bug reports to the original [CPqD OpenFlow 1.3 Software Switch project][cpqdofs13].

# License
This code is released under the BSD license (BSD-like for code from the original Stanford switch).

# Acknowledgments
Thanks to the original [CPqD OpenFlow 1.3 Software Switch][cpqdofs13] contributors. Special thanks to [Eder Leão Fernandes][ederlf], for contributing to the integration between this software switch and the ns-3 simulator.

# Contact
Feel free to subscribe to [our mailing list at Google groups][group] and provide some feedback, give us suggestions, interact with other users, or to just say hello!

[ofp13]: https://www.opennetworking.org/sdn-resources/technical-library
[cpqdofs13]: https://github.com/CPqD/ofsoftswitch13
[ofswitch13]: https://github.com/ljerezchaves/ofswitch13
[ns-3]: https://www.nsnam.org
[ederlf]: https://github.com/ederlf
[group]: https://groups.google.com/forum/#!forum/ofswitch13-users
