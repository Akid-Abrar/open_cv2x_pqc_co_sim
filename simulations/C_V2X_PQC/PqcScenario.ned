package lte.simulations.C_V2X_PQC;


import lte.simulations.C_V2X_PQC.PqcRsu;

import org.car2x.veins.modules.mobility.traci.TraCIScenarioManagerLaunchd;
import org.car2x.veins.base.modules.BaseWorldUtility;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator; // <-- Add this import

import lte.world.radio.LteChannelControl;
import lte.epc.PgwStandardSimplified;
import lte.corenetwork.binder.LteBinder;
import lte.corenetwork.deployer.LteDeployer;
import lte.corenetwork.nodes.eNodeB;
import lte.corenetwork.nodes.Ue;

network PqcScenario
{
    parameters:
        double playgroundSizeX @unit(m);
        double playgroundSizeY @unit(m);
        double playgroundSizeZ @unit(m);
        @display("bgb=1000,1000");

    submodules:
        configurator: IPv4NetworkConfigurator {
            @display("p=50,450");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = parent.playgroundSizeX;
                playgroundSizeY = parent.playgroundSizeY;
                playgroundSizeZ = parent.playgroundSizeZ;
                @display("p=50,50");
        }
        manager: TraCIScenarioManagerLaunchd {
            @display("p=50,150");
        }
        channelControl: LteChannelControl {
            @display("p=50,250");
        }
        binder: LteBinder {
            @display("p=50,350");
        }
        deployer: LteDeployer { // <-- ADD THIS MODULE
            @display("p=50,450");
        }
        rsu: PqcRsu {
            @display("p=500,500");
        }
}