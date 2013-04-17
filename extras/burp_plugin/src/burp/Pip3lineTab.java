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
import java.awt.GridLayout;
import javax.swing.SwingConstants;
import javax.xml.bind.DatatypeConverter;
import javax.swing.JTextField;
import javax.swing.border.TitledBorder;
import java.awt.BorderLayout;
import javax.swing.JCheckBox;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;

public class Pip3lineTab extends JPanel implements ITab {
	private JSpinner portSpinner;
	private JFormattedTextField separatorTextField;
	
	public Pip3lineTab() {
		setLayout(new BorderLayout(0, 0));
		
		mainPanel = new JPanel();
		add(mainPanel, BorderLayout.NORTH);
		mainPanel.setLayout(new GridLayout(2, 1, 0, 0));
		
		confPanel = new JPanel();
		mainPanel.add(confPanel);
		confPanel.setBorder(new TitledBorder(null, "Pip3line connection configuration", TitledBorder.LEADING, TitledBorder.TOP, null, null));
		GridBagLayout gbl_confPanel = new GridBagLayout();
		gbl_confPanel.columnWidths = new int[]{220, 220, 0};
		gbl_confPanel.rowHeights = new int[]{20, 20, 0};
		gbl_confPanel.columnWeights = new double[]{0.0, 0.0, Double.MIN_VALUE};
		gbl_confPanel.rowWeights = new double[]{0.0, 0.0, Double.MIN_VALUE};
		confPanel.setLayout(gbl_confPanel);
		
		hostnameLabel = new JLabel("Hostname");
		hostnameLabel.setHorizontalAlignment(SwingConstants.CENTER);
		GridBagConstraints gbc_hostnameLabel = new GridBagConstraints();
		gbc_hostnameLabel.fill = GridBagConstraints.BOTH;
		gbc_hostnameLabel.insets = new Insets(0, 0, 5, 5);
		gbc_hostnameLabel.gridx = 0;
		gbc_hostnameLabel.gridy = 0;
		confPanel.add(hostnameLabel, gbc_hostnameLabel);
		
		hostnameTextField = new JTextField();
		hostnameTextField.setHorizontalAlignment(SwingConstants.CENTER);
		hostnameTextField.setText("127.0.0.1");
		GridBagConstraints gbc_hostnameTextField = new GridBagConstraints();
		gbc_hostnameTextField.fill = GridBagConstraints.BOTH;
		gbc_hostnameTextField.insets = new Insets(0, 0, 5, 0);
		gbc_hostnameTextField.gridx = 1;
		gbc_hostnameTextField.gridy = 0;
		confPanel.add(hostnameTextField, gbc_hostnameTextField);
		hostnameTextField.setColumns(10);
		
		JLabel portLabel = new JLabel("Server Port");
		GridBagConstraints gbc_portLabel = new GridBagConstraints();
		gbc_portLabel.fill = GridBagConstraints.BOTH;
		gbc_portLabel.insets = new Insets(0, 0, 0, 5);
		gbc_portLabel.gridx = 0;
		gbc_portLabel.gridy = 1;
		confPanel.add(portLabel, gbc_portLabel);
		portLabel.setHorizontalAlignment(SwingConstants.CENTER);
		
		portSpinner = new JSpinner();
		GridBagConstraints gbc_portSpinner = new GridBagConstraints();
		gbc_portSpinner.fill = GridBagConstraints.BOTH;
		gbc_portSpinner.gridx = 1;
		gbc_portSpinner.gridy = 1;
		confPanel.add(portSpinner, gbc_portSpinner);
		portSpinner.setModel(new SpinnerNumberModel(45632, 1, 65535, 1));
		portSpinner.setEditor(new JSpinner.NumberEditor(portSpinner,"#"));
		
		miscPanel = new JPanel();
		miscPanel.setBorder(new TitledBorder(null, "Block format", TitledBorder.LEADING, TitledBorder.TOP, null, null));
		mainPanel.add(miscPanel);
		GridBagLayout gbl_miscPanel = new GridBagLayout();
		gbl_miscPanel.columnWidths = new int[]{440, 0};
		gbl_miscPanel.rowHeights = new int[]{23, 23, 20, 20, 0};
		gbl_miscPanel.columnWeights = new double[]{0.0, Double.MIN_VALUE};
		gbl_miscPanel.rowWeights = new double[]{0.0, 0.0, 0.0, 0.0, Double.MIN_VALUE};
		miscPanel.setLayout(gbl_miscPanel);
		
		decodeCheckBox = new JCheckBox("Base64 decode input");
		decodeCheckBox.setAlignmentX(Component.CENTER_ALIGNMENT);
		decodeCheckBox.setSelected(true);
		GridBagConstraints gbc_decodeCheckBox = new GridBagConstraints();
		gbc_decodeCheckBox.insets = new Insets(0, 0, 5, 0);
		gbc_decodeCheckBox.gridx = 0;
		gbc_decodeCheckBox.gridy = 0;
		miscPanel.add(decodeCheckBox, gbc_decodeCheckBox);
		
		encodeCheckBox = new JCheckBox("Base64 encode output");
		encodeCheckBox.setSelected(true);
		encodeCheckBox.setAlignmentX(Component.CENTER_ALIGNMENT);
		GridBagConstraints gbc_encodeCheckBox = new GridBagConstraints();
		gbc_encodeCheckBox.insets = new Insets(0, 0, 5, 0);
		gbc_encodeCheckBox.gridx = 0;
		gbc_encodeCheckBox.gridy = 1;
		miscPanel.add(encodeCheckBox, gbc_encodeCheckBox);
		
		JLabel lblSeparator = new JLabel("Separator");
		lblSeparator.setAlignmentX(Component.CENTER_ALIGNMENT);
		GridBagConstraints gbc_lblSeparator = new GridBagConstraints();
		gbc_lblSeparator.insets = new Insets(0, 0, 5, 0);
		gbc_lblSeparator.gridx = 0;
		gbc_lblSeparator.gridy = 2;
		miscPanel.add(lblSeparator, gbc_lblSeparator);
		lblSeparator.setHorizontalAlignment(SwingConstants.CENTER);
		
		separatorTextField = new JFormattedTextField();
		separatorTextField.setColumns(4);
		GridBagConstraints gbc_separatorTextField = new GridBagConstraints();
		gbc_separatorTextField.gridx = 0;
		gbc_separatorTextField.gridy = 3;
		miscPanel.add(separatorTextField, gbc_separatorTextField);
		separatorTextField.setHorizontalAlignment(SwingConstants.CENTER);
		separatorTextField.setText("0A");
	}

	/**
	 * 
	 */
	private static final long serialVersionUID = -2320519714434728871L;
	private JPanel confPanel;
	private JLabel hostnameLabel;
	private JTextField hostnameTextField;
	private JPanel mainPanel;
	private JPanel miscPanel;
	private JCheckBox decodeCheckBox;
	private JCheckBox encodeCheckBox;

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
		portSpinner.setValue((Object)new Integer(val));
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
}
