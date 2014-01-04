/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

package burp;

import java.io.PrintWriter;

public class BurpExtender implements IBurpExtender {

//	private IExtensionHelpers helpers;
	private Pip3lineTab tab;
	
	private static IBurpExtenderCallbacks burp;
	private static PayloadProcessor processor;
	private static PrintWriter errOut;
	private static PrintWriter stdOut;
	
	public BurpExtender() {

		tab = new Pip3lineTab();
		processor = null;
		System.out.print("\nPip3line plugin initialised\n");
	}


	@Override
	public void registerExtenderCallbacks(IBurpExtenderCallbacks callbacks) {
//		helpers = callbacks.getHelpers();
		errOut = new PrintWriter(callbacks.getStderr());
		stdOut = new PrintWriter(callbacks.getStdout());
		processor = new PayloadProcessor(tab, callbacks);
		callbacks.registerIntruderPayloadProcessor(processor);
		callbacks.registerContextMenuFactory(new Pip3lineContextMenu(tab, callbacks));
		callbacks.setExtensionName("Pip3line plugin");
		callbacks.addSuiteTab(tab);
		burp = callbacks;
		alert("Pip3line plugin Initialized");
	}

	public static final void alert(String message) {
		burp.issueAlert(message);
		stdOut.println(message);
	}
	
	public static final void error(String message) {
		burp.issueAlert(message);
		errOut.println(message);
	}
	
	public static final void message (String message) {
		stdOut.println(message);
	}
	

}
