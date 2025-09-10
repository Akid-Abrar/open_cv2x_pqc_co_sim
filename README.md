1. Create a new workspace in OMNeT++
2. Clone the updated opencv2x project from https://github.com/Akid-Abrar/open_cv2x_pqc_co_sim.git
3. Clone the updated veins project from https://github.com/Akid-Abrar/veins_cv2x_pqc.git
4. Install INET 3.6.6 as per section 1.1.2 of this document
5. Add the INET and updated veins in the Project References
6. Install liboqs from https://openquantumsafe.org/liboqs/getting-started.html
7. To add libopqs in the project, go to Properties -> OMNeT++ -> Makemake .Then select source and 	go to Options -> Link. Click the add button and type oqs in the diaglogue box and press ok. 	Add crypto and pthread similarly. Finally, go to Preview -> add ( -I/usr/local/include)
