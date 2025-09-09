package lte.apps.mode4App;

import lte.apps.mode4App.Mode4BaseApp;

// NED definition for the RSU application
simple Mode4RSUApp like Mode4BaseApp
{
    parameters:
        
        @class(Mode4RSUApp);
        @display("i=block/app");
        
        int socketPort = default(9011);
        double socketPollInterval @unit(s) = default(0.05s); 
        int slDurationMs = default(10);
        int slPriority = default(1);
    	int slLcid = default(5);
        
        @signal[rsuReceivedMsg];
        @statistic[rsuReceivedMsg](title="Messages Received at RSU"; unit=""; source="rsuReceivedMsg"; record=sum,vector);
        @signal[rsuVerifiedMsg]; 
        @statistic[rsuVerifiedMsg](title="Total Msg Verified at RSU"; unit=""; source="rsuVerifiedMsg"; record=sum,vector);  
     	@signal[cbr];
        @statistic[cbr](title="Channel Busy Ratio"; unit=""; source="cbr"; record=vector); 
        @signal[numread];
        @statistic[numread](title="Socket"; unit=""; source="numread"; record=sum,vector); 
        @signal[numread0];
        @statistic[numread0](title="Socket"; unit=""; source="numread0"; record=sum,vector); 
        @signal[numread1];
        @statistic[numread1](title="Socket"; unit=""; source="numread1"; record=sum,vector); 
        @signal[numBroadcasted];
        @statistic[numBroadcasted](title="Socket"; unit=""; source="numBroadcasted"; record=sum,vector); 
    gates:
        input lowerGateIn;
        output lowerGateOut;
}