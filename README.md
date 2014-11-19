<<<<<<< HEAD
PhArS: Phraseblock Architecture Simulator

# README #

This README documents the steps necessary to get this application up and running.

### What is this repository for? ###

* Milad's thesis research project (phraseblock execution simulator).

### How do I get set up? ###

* Setup Pintool
* Build the simulator

### Who do I talk to? ###

* Milad Mohammad: milad-at-alumni-dot-stanford-dot-edu
=======
cache_spice
===========

A cache model that uses HSPICE to get better estimates for energy and delay.

You have to have `hspice` in your PATH to be able to use this cache model. `mod_setup.bash` is an example script that works on my system, but may or may not work for you.

To run, modify the appropriate parameters in scripts/config.yaml, then:

```
$ cd scripts
$ python cache.py config.yaml
```

The code is (hopefully) commented fine enough so that you can easily get something else you need such as delay/tag array delay etc. Also, the cell model/wire models are all configurable via the yaml file. For more, see the config file.
>>>>>>> 90e3363bacd219dfaa6db1af55e394d7817221aa
