package lte.simulations.C_V2X_PQC;

// No package declaration needed as this file is not in a standard package location

import lte.C_V2X_PQC.MyVehicleApp; // Import your custom app from its package
import lte.stack.phy.ILteNic;
import org.car2x.veins.base.modules.IMobility;
import org.car2x.veins.base.modules.IBaseApplLayer;

// This module combines the application, mobility, and NIC for a vehicle
module PqcVehicle
{
    parameters:
        @networkNode();
        @display("i=car;bgb=400,518");
        string veinsmobilityType = default("org.car2x.veins.modules.mobility.traci.TraCIMobility");
        string applType = default("lte.C_V2X_PQC.MyVehicleApp");
        string nicType = default("LteNicVUeMode4");
        string nodeType = "UE";  // DO NOT CHANGE
        int masterId = default(0); // Not yet removed from SimuLTE for Mode 4 simulations
        int macNodeId = default(0); // TODO: this is not a real parameter
        int macCellId = default(0); // TODO: this is not a real parameter
        //# D2D capability
        bool d2dCapable = default(false);
        bool mode4D2D = default(false);

    gates:
        input radioIn @directIn;

    submodules:
        veinsmobility: <veinsmobilityType> like org.car2x.veins.base.modules.IMobility {
            @display("p=130,172;i=block/cogwheel");
        }
        appl: <applType> like lte.apps.mode4App.Mode4BaseApp {
            @display("p=60,50");
        }
        lteNic: <nicType> like ILteNic {
            nodeType = nodeType;
            d2dCapable = d2dCapable;
            @display("p=250,407");
        }
    connections allowunconnected:
        lteNic.upperLayerOut --> appl.lowerGateIn;
        lteNic.upperLayerIn <-- appl.lowerGateOut;
        lteNic.radioIn <-- radioIn;
}