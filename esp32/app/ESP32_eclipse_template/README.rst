ESP-IDF Eclipse Template demo
========================

This is the demo to use Eclipse.

Preparation
	install esp-idf (https://github.com/espressif/esp-idf)
	check $PATH
	check $IDF_PATH

Config steps on Ubuntu Linux:
    1. edit include.xml
       change ESP32_eclipse_template to your PATH
    2. start eclipse
    3. config make-targets for (if exist copy them from previous project)
              menuconfig
              all (Target "-j8 all")
              clear
              flash (Command gnome-terminal -x make)
              monitor (Command gnome-terminal -x make)
              flash monitor (Command gnome-terminal -x make)
    4. import include.xml 
             Project Explorer -> Properties -> C/C++ General -> Paths and Symbols
		
		Includes -> Import Serttings
		Browse (Open File include.xml)
		
		-> Finish
     5. Use Make-Targets menuconfig
     6. Use Make-Targets all, flash and monitor
     7. enjoy

	




