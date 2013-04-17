/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

package burp;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import java.net.InetAddress;
import java.net.Socket;
import java.util.ArrayList;

public class BurpExtender implements IBurpExtender, IIntruderPayloadProcessor {

	private IExtensionHelpers helpers;

	private IBurpExtenderCallbacks burp;
	private byte separator;
	private int port;
	private String hostName;
	private boolean urlEncodeOutput;
	private Socket socket;
	private OutputStream pipeOut;
	private InputStream pipeIn;
	private Pip3lineTab tab;
	private boolean decode;
	private boolean encode;
	
	public BurpExtender() {
		separator = 0x0A;
		port = 45632;
		hostName = "127.0.0.1";
		
		decode = true;
		encode = true;
		
		socket = null;
		pipeOut = null;
		pipeIn = null;
		urlEncodeOutput = true;
		tab = new Pip3lineTab();
		tab.setPort(port);
		tab.setSeparator(separator);
		tab.setDecode(decode);
		tab.setEncode(encode);
		tab.setHostname(hostName);
		System.out.print("\nPip3line plugin initialised\n");
	}
	
	@Override
	public String getProcessorName() {
		return "Pip3line processing";
	}

	@Override
	public byte[] processPayload(byte[] currentPayload, byte[] originalPayload,
			byte[] baseValue) {
		
		return internalProcessing(currentPayload);
	}

	@Override
	public void registerExtenderCallbacks(IBurpExtenderCallbacks callbacks) {
		helpers = callbacks.getHelpers();
		callbacks.registerIntruderPayloadProcessor(this);
		callbacks.setExtensionName("Pip3line plugin");
		callbacks.addSuiteTab(tab);
		burp = callbacks;
		alerting("Initialized");
	}

	private void alerting(String message) {
		burp.issueAlert(message);
		System.out.println(message);
	}
	
	private byte[] internalProcessing(byte[] currentPayload) {
		separator = tab.getSeparator();
		port = tab.getPort();
		hostName = tab.getHostname();
		urlEncodeOutput = true;

		decode = tab.isDecode();
		encode = tab.isEncode();
		
		byte[] returnValue = null;
		byte[] separators = { separator };

		try {
			if (socket == null || socket.isClosed()) {
				socket = new Socket(InetAddress.getByName(hostName), port);
				pipeOut = socket.getOutputStream();
				pipeIn = socket.getInputStream();
			}
            returnValue = null;
            byte[] data = helpers.urlDecode(currentPayload);
            if (encode) {
            	data = helpers.stringToBytes(helpers.base64Encode(data));
            }
            
            
			try {
				pipeOut.write(data);
				pipeOut.write(separators);
				
		//		System.out.println("Data block send: ".concat(helpers.bytesToString(data)));
				
				int byteRead = pipeIn.read();
				ArrayList<Byte> returnedVals = new ArrayList<Byte>();
				while (byteRead != -1 && byteRead != separator) {
					returnedVals.add((byte) byteRead);
					byteRead = pipeIn.read();
				}
			
				returnValue = new byte[returnedVals.size()];
				
				for (int i = 0; i < returnedVals.size(); i++) {
					returnValue[i] = returnedVals.get(i).byteValue();
				}
				
			//	System.out.println("Data block read: ".concat(helpers.bytesToString(returnValue)));
				if (decode) {
					returnValue  = helpers.base64Decode(returnValue);
				}
				if (urlEncodeOutput) {
					returnValue  = helpers.urlEncode(returnValue);
				}
				
				socket.close();
				
			} catch (IOException e) {
				alerting("Pipe error: ".concat(e.getMessage()));
			}
			try {
				pipeOut.close();
				pipeIn.close();
			} catch (IOException e) {
				alerting("Pipe closing error: ".concat(e.getMessage()));
			}
			
		} catch (IOException e) {
			alerting("Socket opening error: ".concat(e.getMessage()));
		}
		return returnValue;
		
	}
}
