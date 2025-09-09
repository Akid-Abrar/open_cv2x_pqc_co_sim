package lte.simulations.C_V2X_PQC;

import lte.C_V2X_PQC.MyRSUApp; // Import your custom app from its package
import inet.node.inet.StandardHost;
import lte.stack.phy.ILteNic;
import org.car2x.veins.base.modules.IBaseApplLayer;

// Defines the RSU as a stationary StandardHost with an LTE NIC
module PqcRsu extends StandardHost
{
    parameters:
        @display("i=veins/sign_rsz;bgb=400,518");
        string applType = default("C_V2X_PQC.MyRSUApp");
        string nicType = default("LteNicVUeMode4");
        *.numApps = 1;
        *.app[0].typename = applType;
        *.wlan[0].typename = nicType;
}