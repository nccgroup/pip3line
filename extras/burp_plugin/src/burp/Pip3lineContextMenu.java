/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

package burp;

import java.awt.event.ActionEvent;
import java.io.IOException;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import javax.swing.AbstractAction;
import javax.swing.JMenuItem;

public class Pip3lineContextMenu implements IContextMenuFactory {
	
	private ArrayList<JMenuItem> itemList;
	private IExtensionHelpers helpers;
	private int udpSizeLimit;
	private boolean useUDP = false;
	private byte separator = 0x0A;
	private Pip3lineTab tab;
	public Pip3lineContextMenu(Pip3lineTab configTab,IBurpExtenderCallbacks callbacks) {
		helpers = callbacks.getHelpers();
		itemList = new ArrayList<JMenuItem>();
		udpSizeLimit = 65507; // UDP limit
		tab = configTab;
		try {
			tab.setSTHostname(InetAddress.getByName("localhost"));
		} catch (UnknownHostException e) {
			BurpExtender.error("Unkown host address exception (should never happen for localhost)");
		}
		
		
		tab.setSTPort(40000);
		tab.setSTSeparator(separator);
		tab.setSTEncode(true);
	}

	
	@Override
	public List<JMenuItem> createMenuItems(IContextMenuInvocation arg0) {
		itemList.clear();
		int tool = arg0.getToolFlag();
//    	System.out.print("Tools calling:");
//    	System.out.print(tool);
		if (tool == IBurpExtenderCallbacks.TOOL_INTRUDER ||
				tool == IBurpExtenderCallbacks.TOOL_PROXY ||
				tool == IBurpExtenderCallbacks.TOOL_REPEATER ||
				tool == IBurpExtenderCallbacks.TOOL_TARGET ||
				tool == IBurpExtenderCallbacks.TOOL_COMPARER) {
			
			//System.out.println(" ==> Accepted");
			JMenuItem sendToPip3line = new JMenuItem(new SendToPip3lineAction(arg0));
			itemList.add(sendToPip3line);
			return itemList;
		} else {
			//System.out.println(" ==> Rejected");
		}
		
		return null;
	}
	
	class SendToPip3lineAction extends AbstractAction {
		private static final long serialVersionUID = -3036431674498504569L;

		private int[] selection;
		private byte[] data;
		private boolean request;
	    public SendToPip3lineAction(IContextMenuInvocation contextMenu) {
	        super("Send to Pip3line");
	        putValue(SHORT_DESCRIPTION, "Send data to Pip3line");
	        byte contextInv = contextMenu.getInvocationContext();
	        request = contextInv == IContextMenuInvocation.CONTEXT_MESSAGE_EDITOR_REQUEST ||
	        		contextInv == IContextMenuInvocation.CONTEXT_MESSAGE_VIEWER_REQUEST ||
	        		contextInv == IContextMenuInvocation.CONTEXT_INTRUDER_PAYLOAD_POSITIONS;
	        
	        IHttpRequestResponse[] messages = contextMenu.getSelectedMessages();
	        if (messages != null && messages.length > 0) {
//	        	System.out.print("Got messages :");
//	        	System.out.println(messages.length);
	        	if (request) {
	        		data = messages[0].getRequest();
	        	}
	        	else {
	        		data = messages[0].getResponse();
	        	}
	        	
//	        	System.out.print("Data length:");
//	        	System.out.println(data.length);
	        	
	        	selection = contextMenu.getSelectionBounds();
	        	if (selection[0] != selection[1]) {
	        		data = Arrays.copyOfRange(data,selection[0], selection[1]);
	        	}
	        	if (tab.isSTEncode())
	        		data = helpers.base64Encode(data).getBytes();
//	        	
//	        	System.out.print("Selection :");
//	        	System.out.print(selection[0]);
//	        	System.out.print("-");
//	        	System.out.println(selection[1]);
	        }
	    }
	    public void actionPerformed(ActionEvent e) {
	    	if (useUDP) {
	    		sendOverUDP();
	    	} else {
	    		sendOverTCP();
	    	}
	    }
	    
	    private void sendOverUDP() {
	    	DatagramSocket clientSocket;
			try {
				clientSocket = new DatagramSocket();
		        
	        	if (data.length > udpSizeLimit) { // UDP limit
	        		BurpExtender.error("SendToPip3line: packet too large for UDP, truncating");
	        		data = Arrays.copyOfRange(data,0,udpSizeLimit);
	        	}

		        DatagramPacket sendPacket = new DatagramPacket(data, data.length, tab.getSTHostname(), tab.getSTPort());
		        clientSocket.send(sendPacket);
			} catch (SocketException e1) {
				BurpExtender.error(e1.getMessage());
			} catch (IOException e1) {
				BurpExtender.error("Error while sending packet to pip3line".concat(e1.getMessage()));
			}
	    }
	    
	    private void sendOverTCP() {
			try {
				Socket socket = new Socket(tab.getSTHostname(), tab.getSTPort());;
		
				OutputStream pipeOut = socket.getOutputStream();

				try {
					pipeOut.write(data);
					pipeOut.write(tab.getSTSeparator());
					socket.close();
					
				} catch (IOException e) {
					BurpExtender.error("Error while sending packet to pip3line: ".concat(e.getMessage()));
				}
				try {
					pipeOut.close();
				} catch (IOException e) {
					BurpExtender.error("Pipe closing error: ".concat(e.getMessage()));
				}
				
			} catch (IOException e) {
				BurpExtender.error("Socket opening error: ".concat(e.getMessage()));
			}
	    }
	}

}
