package lte.simulations.Mode4;

import inet.node.inet.StandardHost;
import inet.mobility.static.StationaryMobility;

import lte.stack.phy.ILteNic;
import lte.stack.phy.LteNicUe;
import lte.stack.phy.LteNicUeD2D;
import lte.stack.phy.LteNicVUeMode4;
import lte.apps.mode4App.*;
import org.car2x.veins.base.modules.*;

module RSU
{
    parameters:
        @networkNode();
        @display("i=veins/sign/yellowdiamond;is=vs");

        string nodeType = "UE";  // DO NOT CHANGE
        int masterId = default(0); // Not yet removed from SimuLTE for Mode 4 simulations
        int macNodeId = default(0); // TODO: this is not a real parameter
        int macCellId = default(0); // TODO: this is not a real parameter
        
        string applType = default("lte.apps.mode4App.Mode4RSUApp"); //type of the application layer
        string veinsmobilityType = default("org.car2x.veins.modules.mobility.traci.TraCIMobility"); //type of the mobility module
        //string veinsmobilityType = default("inet.mobility.static.StationaryMobility");

        //# D2D capability
        bool d2dCapable = default(true);
        bool mode4D2D = default(true);
        
		string nicType = default("LteNicVUeMode4");

    gates:
        input radioIn @directIn;     // connection to master

    submodules:
        
//		veinsmobility: <veinsmobilityType> like org.car2x.veins.base.modules.IMobility {
//            parameters:
//                @display("p=130,172;i=block/cogwheel");
//        }
        veinsmobility: BaseMobility {
            parameters:
                @display("p=130,172;i=block/cogwheel");
        }
        appl: <applType> like lte.apps.mode4App.Mode4BaseApp {
            parameters:
                @display("p=60,50");
        }
        
        // NOTE: instance must be named "lteNic"
        lteNic: <nicType> like ILteNic {
            nodeType = nodeType;
            d2dCapable = d2dCapable;
            @display("p=250,407");
        }
    connections allowunconnected:
        //# 
        //# Internal TCP/UDP applications connections with IP stack
        //#

        lteNic.upperLayerOut --> appl.lowerGateIn;
        lteNic.upperLayerIn <-- appl.lowerGateOut;

        lteNic.radioIn <-- radioIn;
}