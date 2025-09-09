// 
//                           SimuLTE
// 
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself, 
// and cannot be removed from it.
//
package lte.simulations.Mode4;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.ethernet.Eth10G;

import lte.world.radio.LteChannelControl;
import lte.epc.PgwStandardSimplified;
import lte.corenetwork.binder.LteBinder;
import lte.corenetwork.deployer.LteDeployer;
import lte.corenetwork.nodes.eNodeB;
import lte.corenetwork.nodes.Ue;
//import lte.simulations.Mode4.CarNonIp;
//import lte.corenetwork.nodes.cars.CarNonIp;

//import org.car2x.veins.subprojects.veins_inet.VeinsInetManager;
import org.car2x.veins.modules.mobility.traci.TraCIScenarioManager*;// Replace VeinsInetManager with TraCIScenarioManager
import org.car2x.veins.base.modules.BaseWorldUtility;

//import lte.simulations.Mode4.RSU;

network Highway
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        @display("bgb=2500,2500");
        int numRsu = default(1);

    submodules:

//        //# Veins manager module
//        veinsManager: VeinsInetManager {
//            @display("p=50,200;is=s");
//        }

        // Veins scenario manager module
        configurator: IPv4NetworkConfigurator {
            @display("p=50,312;is=s");
        }
        manager: TraCIScenarioManagerLaunchd {
            @display("p=50,200;is=s");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = veins_eval_by_version(veins_omnetpp_buildnum(), "playgroundSizeX", 1525, "parent.playgroundSizeX");
                playgroundSizeY = veins_eval_by_version(veins_omnetpp_buildnum(), "playgroundSizeY", 1525, "parent.playgroundSizeY");
                playgroundSizeZ = veins_eval_by_version(veins_omnetpp_buildnum(), "playgroundSizeZ", 1525, "parent.playgroundSizeZ");
                @display("p=30,0;i=misc/globe");
        }

        //        # LTE modules
        channelControl: LteChannelControl {
            @display("p=50,25;is=s");
        }
        binder: LteBinder {
            @display("p=50,140;is=s");
        }
        deployer: LteDeployer {
            @display("p=50,259;is=s");
        }
        rsu[numRsu]: RSU {
            @display("p=366,240"); // Position RSU in the center
        }
}
