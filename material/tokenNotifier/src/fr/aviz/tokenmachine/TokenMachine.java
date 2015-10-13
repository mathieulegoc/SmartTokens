package fr.aviz.tokenmachine;

import jssc.SerialPort;
import jssc.SerialPortException;

public class TokenMachine {
	private SerialPort serialPort = null;

	public TokenMachine() {
		serialPort = new SerialPort("COM10");
		try {
			serialPort.openPort();
			serialPort.setParams(9600, 8, 1, 0);
			
			// System.out.println("Port closed: " + serialPort.closePort());
		} catch (SerialPortException ex) {
			System.out.println(ex);
		}
	}

	public boolean DropToken(int id) {
		try {
			serialPort.writeByte((byte) id);
			return true;
		} catch (SerialPortException ex) {
			System.out.println(ex);
		}
		return false;
	}

	public boolean Close() {
		try {
			serialPort.closePort();
			return true;
		} catch (SerialPortException ex) {
			System.out.println(ex);
		}
		return false;
	}
	
	public boolean isConnected(){
		return serialPort.isOpened();
	}
}