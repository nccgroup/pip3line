/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

package burp;

import java.awt.Component;

import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.JFormattedTextField;
import javax.swing.SwingConstants;
import javax.xml.bind.DatatypeConverter;
import javax.swing.JTextField;
import javax.swing.border.TitledBorder;

import java.awt.BorderLayout;

import javax.swing.JCheckBox;

import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;

import javax.swing.border.LineBorder;

import java.awt.Color;
import java.net.InetAddress;
import java.net.UnknownHostException;

import javax.swing.JTextPane;
import javax.swing.BoxLayout;

public class Pip3lineTab extends JPanel implements ITab {
	private JSpinner portSpinner;
	private JFormattedTextField separatorTextField;
	private JCheckBox sendToBase64EncodeBlock;
	public Pip3lineTab() {
		setLayout(new BorderLayout(0, 0));
		
		mainPanel = new JPanel();
		add(mainPanel, BorderLayout.NORTH);
		mainPanel.setLayout(new BoxLayout(mainPanel, BoxLayout.Y_AXIS));
		
		confPanel = new JPanel();
		mainPanel.add(confPanel);
		confPanel.setBorder(new TitledBorder(new LineBorder(new Color(184, 207, 229)), "Pip3line Intruder Payload processor configuration", TitledBorder.LEADING, TitledBorder.TOP, null, null));
		GridBagLayout gbl_confPanel = new GridBagLayout();
		gbl_confPanel.columnWidths = new int[]{220, 220, 0};
		gbl_confPanel.rowHeights = new int[]{20, 20, 0, 0, 0, 0};
		gbl_confPanel.columnWeights = new double[]{0.0, 0.0, Double.MIN_VALUE};
		gbl_confPanel.rowWeights = new double[]{0.0, 0.0, 0.0, 0.0, 0.0, Double.MIN_VALUE};
		confPanel.setLayout(gbl_confPanel);
		
		hostnameLabel = new JLabel("Hostname");
		hostnameLabel.setHorizontalAlignment(SwingConstants.LEFT);
		GridBagConstraints gbc_hostnameLabel = new GridBagConstraints();
		gbc_hostnameLabel.anchor = GridBagConstraints.WEST;
		gbc_hostnameLabel.fill = GridBagConstraints.BOTH;
		gbc_hostnameLabel.insets = new Insets(0, 0, 5, 5);
		gbc_hostnameLabel.gridx = 0;
		gbc_hostnameLabel.gridy = 0;
		confPanel.add(hostnameLabel, gbc_hostnameLabel);
		
		hostnameTextField = new JTextField();
		hostnameTextField.setHorizontalAlignment(SwingConstants.CENTER);
		hostnameTextField.setText("127.0.0.1");
		GridBagConstraints gbc_hostnameTextField = new GridBagConstraints();
		gbc_hostnameTextField.anchor = GridBagConstraints.WEST;
		gbc_hostnameTextField.insets = new Insets(0, 0, 5, 0);
		gbc_hostnameTextField.gridx = 1;
		gbc_hostnameTextField.gridy = 0;
		confPanel.add(hostnameTextField, gbc_hostnameTextField);
		hostnameTextField.setColumns(10);
		
		JLabel portLabel = new JLabel("Server Port");
		GridBagConstraints gbc_portLabel = new GridBagConstraints();
		gbc_portLabel.anchor = GridBagConstraints.WEST;
		gbc_portLabel.fill = GridBagConstraints.BOTH;
		gbc_portLabel.insets = new Insets(0, 0, 5, 5);
		gbc_portLabel.gridx = 0;
		gbc_portLabel.gridy = 1;
		confPanel.add(portLabel, gbc_portLabel);
		portLabel.setHorizontalAlignment(SwingConstants.LEFT);
		
		portSpinner = new JSpinner();
		GridBagConstraints gbc_portSpinner = new GridBagConstraints();
		gbc_portSpinner.anchor = GridBagConstraints.WEST;
		gbc_portSpinner.insets = new Insets(0, 0, 5, 0);
		gbc_portSpinner.gridx = 1;
		gbc_portSpinner.gridy = 1;
		confPanel.add(portSpinner, gbc_portSpinner);
		portSpinner.setModel(new SpinnerNumberModel(45632, 1, 65535, 1));
		portSpinner.setEditor(new JSpinner.NumberEditor(portSpinner,"#"));
		
		JLabel lblSeparator = new JLabel("Block separator");
		GridBagConstraints gbc_lblSeparator = new GridBagConstraints();
		gbc_lblSeparator.anchor = GridBagConstraints.WEST;
		gbc_lblSeparator.insets = new Insets(0, 0, 5, 5);
		gbc_lblSeparator.gridx = 0;
		gbc_lblSeparator.gridy = 2;
		confPanel.add(lblSeparator, gbc_lblSeparator);
		lblSeparator.setAlignmentX(Component.CENTER_ALIGNMENT);
		lblSeparator.setHorizontalAlignment(SwingConstants.LEFT);
		
		separatorTextField = new JFormattedTextField();
		GridBagConstraints gbc_separatorTextField = new GridBagConstraints();
		gbc_separatorTextField.anchor = GridBagConstraints.WEST;
		gbc_separatorTextField.insets = new Insets(0, 0, 5, 0);
		gbc_separatorTextField.gridx = 1;
		gbc_separatorTextField.gridy = 2;
		confPanel.add(separatorTextField, gbc_separatorTextField);
		separatorTextField.setColumns(4);
		separatorTextField.setHorizontalAlignment(SwingConstants.CENTER);
		separatorTextField.setText("0A");
		
		decodeCheckBox = new JCheckBox("Base64 decode input");
		GridBagConstraints gbc_decodeCheckBox = new GridBagConstraints();
		gbc_decodeCheckBox.anchor = GridBagConstraints.WEST;
		gbc_decodeCheckBox.insets = new Insets(0, 0, 5, 0);
		gbc_decodeCheckBox.gridx = 1;
		gbc_decodeCheckBox.gridy = 3;
		confPanel.add(decodeCheckBox, gbc_decodeCheckBox);
		decodeCheckBox.setAlignmentX(Component.CENTER_ALIGNMENT);
		decodeCheckBox.setSelected(true);
		
		encodeCheckBox = new JCheckBox("Base64 encode output");
		GridBagConstraints gbc_encodeCheckBox = new GridBagConstraints();
		gbc_encodeCheckBox.anchor = GridBagConstraints.WEST;
		gbc_encodeCheckBox.gridx = 1;
		gbc_encodeCheckBox.gridy = 4;
		confPanel.add(encodeCheckBox, gbc_encodeCheckBox);
		encodeCheckBox.setSelected(true);
		encodeCheckBox.setAlignmentX(Component.CENTER_ALIGNMENT);
		
		sendTopanel = new JPanel();
		sendTopanel.setBorder(new TitledBorder(new LineBorder(new Color(184, 207, 229)), "\"Send to Pip3line\" configuration", TitledBorder.LEADING, TitledBorder.TOP, null, null));
		mainPanel.add(sendTopanel);
		GridBagLayout gbl_sendTopanel = new GridBagLayout();
		gbl_sendTopanel.columnWidths = new int[] {220, 220, 0};
		gbl_sendTopanel.rowHeights = new int[] {20, 20, 0, 0, 0, 0};
		gbl_sendTopanel.columnWeights = new double[]{0.0, 0.0, Double.MIN_VALUE};
		gbl_sendTopanel.rowWeights = new double[]{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
		sendTopanel.setLayout(gbl_sendTopanel);
		
		txtpngeneralySpeakingOne = new JTextPane();
		txtpngeneralySpeakingOne.setEditable(false);
		txtpngeneralySpeakingOne.setText("Generaly speaking those parameters don't need to be changed, there are mainly here for testing purposes");
		GridBagConstraints gbc_txtpngeneralySpeakingOne = new GridBagConstraints();
		gbc_txtpngeneralySpeakingOne.insets = new Insets(0, 0, 5, 0);
		gbc_txtpngeneralySpeakingOne.fill = GridBagConstraints.BOTH;
		gbc_txtpngeneralySpeakingOne.gridx = 1;
		gbc_txtpngeneralySpeakingOne.gridy = 0;
		sendTopanel.add(txtpngeneralySpeakingOne, gbc_txtpngeneralySpeakingOne);
		
		lblHostname = new JLabel("Hostname");
		lblHostname.setHorizontalAlignment(SwingConstants.LEFT);
		GridBagConstraints gbc_lblHostname = new GridBagConstraints();
		gbc_lblHostname.insets = new Insets(0, 0, 5, 5);
		gbc_lblHostname.anchor = GridBagConstraints.WEST;
		gbc_lblHostname.gridx = 0;
		gbc_lblHostname.gridy = 1;
		sendTopanel.add(lblHostname, gbc_lblHostname);
		
		sendToHostnametextField = new JTextField();
		sendToHostnametextField.setHorizontalAlignment(SwingConstants.CENTER);
		sendToHostnametextField.setText("127.0.0.1");
		GridBagConstraints gbc_sendToHostnametextField = new GridBagConstraints();
		gbc_sendToHostnametextField.anchor = GridBagConstraints.WEST;
		gbc_sendToHostnametextField.insets = new Insets(0, 0, 5, 0);
		gbc_sendToHostnametextField.gridx = 1;
		gbc_sendToHostnametextField.gridy = 1;
		sendTopanel.add(sendToHostnametextField, gbc_sendToHostnametextField);
		sendToHostnametextField.setColumns(10);
		
		lblPort = new JLabel("Port");
		lblPort.setHorizontalAlignment(SwingConstants.LEFT);
		GridBagConstraints gbc_lblPort = new GridBagConstraints();
		gbc_lblPort.anchor = GridBagConstraints.WEST;
		gbc_lblPort.insets = new Insets(0, 0, 5, 5);
		gbc_lblPort.gridx = 0;
		gbc_lblPort.gridy = 2;
		sendTopanel.add(lblPort, gbc_lblPort);
		
		sentToPortSpinner = new JSpinner();
		sentToPortSpinner.setModel(new SpinnerNumberModel(40003, 1, 65535, 1));
		JSpinner.NumberEditor editor = new JSpinner.NumberEditor(sentToPortSpinner, "#"); 
		sentToPortSpinner.setEditor(editor);
		GridBagConstraints gbc_sentToPortSpinner = new GridBagConstraints();
		gbc_sentToPortSpinner.anchor = GridBagConstraints.WEST;
		gbc_sentToPortSpinner.insets = new Insets(0, 0, 5, 0);
		gbc_sentToPortSpinner.gridx = 1;
		gbc_sentToPortSpinner.gridy = 2;
		sendTopanel.add(sentToPortSpinner, gbc_sentToPortSpinner);
		
		lblSeparator_1 = new JLabel("Separator");
		lblSeparator_1.setHorizontalAlignment(SwingConstants.LEFT);
		GridBagConstraints gbc_lblSeparator_1 = new GridBagConstraints();
		gbc_lblSeparator_1.anchor = GridBagConstraints.WEST;
		gbc_lblSeparator_1.insets = new Insets(0, 0, 5, 5);
		gbc_lblSeparator_1.gridx = 0;
		gbc_lblSeparator_1.gridy = 3;
		sendTopanel.add(lblSeparator_1, gbc_lblSeparator_1);
		
		sentToSepaTextField = new JTextField();
		sentToSepaTextField.setText("0A");
		sentToSepaTextField.setHorizontalAlignment(SwingConstants.CENTER);
		GridBagConstraints gbc_sentToSepaTextField = new GridBagConstraints();
		gbc_sentToSepaTextField.anchor = GridBagConstraints.WEST;
		gbc_sentToSepaTextField.insets = new Insets(0, 0, 5, 0);
		gbc_sentToSepaTextField.gridx = 1;
		gbc_sentToSepaTextField.gridy = 3;
		sendTopanel.add(sentToSepaTextField, gbc_sentToSepaTextField);
		sentToSepaTextField.setColumns(4);
		
		sendToBase64EncodeBlock = new JCheckBox("Base64 encode block");
		GridBagConstraints gbc_sendToBase64EncodeBlock = new GridBagConstraints();
		gbc_sendToBase64EncodeBlock.insets = new Insets(0, 0, 5, 0);
		gbc_sendToBase64EncodeBlock.anchor = GridBagConstraints.WEST;
		gbc_sendToBase64EncodeBlock.gridx = 1;
		gbc_sendToBase64EncodeBlock.gridy = 4;
		sendTopanel.add(sendToBase64EncodeBlock, gbc_sendToBase64EncodeBlock);
	}

	/**
	 * 
	 */
	private static final long serialVersionUID = -2320519714434728871L;
	private JPanel confPanel;
	private JLabel hostnameLabel;
	private JTextField hostnameTextField;
	private JPanel mainPanel;
	private JCheckBox decodeCheckBox;
	private JCheckBox encodeCheckBox;
	private JPanel sendTopanel;
	private JLabel lblHostname;
	private JTextField sendToHostnametextField;
	private JLabel lblPort;
	private JSpinner sentToPortSpinner;
	private JLabel lblSeparator_1;
	private JTextField sentToSepaTextField;
	private JTextPane txtpngeneralySpeakingOne;

	@Override
	public String getTabCaption() {
		return "Pip3line";
	}

	@Override
	public Component getUiComponent() {
		return this;
	}
	
	public int getPort() {
		return ((Integer)portSpinner.getValue()).intValue();
	}
	
	public void setPort(int val) {
		portSpinner.setValue(new Integer(val));
	}
	
	public byte getSeparator() {
		return DatatypeConverter.parseHexBinary(separatorTextField.getText())[0];
	}
	
	public void setSeparator(byte val) {
		
		separatorTextField.setText(String.format("%02X", val));
	}
	
	public boolean isDecode() {
		return decodeCheckBox.isSelected();
	}

	public void setDecode(boolean val) {
		decodeCheckBox.setSelected(val);
	}
	
	public boolean isEncode() {
		return encodeCheckBox.isSelected();
	}
	
	public void setEncode(boolean val) {
		encodeCheckBox.setSelected(val);
	}
	
	public String getHostname() {
		return hostnameTextField.getText();
	}
	
	public void setHostname(String host) {
		hostnameTextField.setText(host);
	}
	
	public InetAddress getSTHostname() throws UnknownHostException {
		return InetAddress.getByName(sendToHostnametextField.getText());
	}
	
	public void setSTHostname(InetAddress val) {
		sendToHostnametextField.setText(val.getHostAddress());
	}
	
	public int getSTPort() {
		return ((Integer)sentToPortSpinner.getValue()).intValue();
	}
	
	public void setSTPort(int val) {
		sentToPortSpinner.setValue(new Integer(val));
	}
	
	public byte getSTSeparator() {
		return DatatypeConverter.parseHexBinary(sentToSepaTextField.getText())[0];
	}
	
	public void setSTSeparator(byte val) {
		sentToSepaTextField.setText(String.format("%02X",val));
	}
	
	public boolean isSTEncode() {
		return sendToBase64EncodeBlock.isSelected();
	}
	
	public void setSTEncode(boolean val) {
		sendToBase64EncodeBlock.setSelected(val);
	}
}
