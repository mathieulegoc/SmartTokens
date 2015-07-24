package fr.aviz.sensingtokens;

import java.net.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.io.*;

public class TokenSocketClient extends Thread {

    private InetAddress iAddress = null;
    private Socket client = null;
    public int socketPort = 11000;
    private DataOutputStream outToServer = null;
    private DataInputStream inFromServer = null;
    private boolean isConnected = false;
    private boolean running = true;
    // private List<MessageFromServer> incomingMessages;
    private ArrayList<TokenEventListener> tokenEventListeners = new ArrayList<TokenEventListener>();

    public TokenSocketClient() {
	// incomingMessages = new ArrayList<MessageFromServer>();
    }

    public void run() {
	while (running) {
	    while (!isConnected) {
		Connect();
		try {
		    Thread.sleep(1000);
		} catch (InterruptedException e) {
		    e.printStackTrace();
		}
	    }
	    while (isConnected) {
		MessageFromServer message = collectMessage();
		if (message != null) {
		    TokenEvent event = new TokenEvent(message.getSenderId(), message.getPayload()[0],
			    message.getPayload()[1]);
		    for (TokenEventListener listener : tokenEventListeners) {
			listener.tokenEventReceived(event);
		    }
		}
		try {
		    Thread.sleep(10);
		} catch (InterruptedException e) {
		    e.printStackTrace();
		}
	    }
	}
    }

    // while (timeout < 2000) {
    // MessageFromServer msg = socket.getLastMessage();
    // if(msg!=null){
    // tokenIdentified = (msg.getMessageType() == MessageTypes.STATUS_DATA &&
    // msg.getPayload()[0] == transitionType) ?
    // true : false;
    // if (tokenIdentified) {
    // tokenId = (int) msg.getSenderId();
    // socket.deleteMessage(msg);
    // break;
    // }
    // } else {
    // timeout += 50;
    // try {
    // Thread.sleep(50);
    // } catch (InterruptedException e) {
    // // TODO Auto-generated catch block
    // e.printStackTrace();
    // }
    // }
    // }
    // if (tokenId == -1)
    // System.out.println("the dropped token couldn't be retrieved!\n");
    // else
    // System.out.println("token identified with id="+tokenId+"\n");

    public void addTokenEventListener(TokenEventListener eventListener) {
	tokenEventListeners.add(eventListener);
    }

    private synchronized MessageFromServer collectMessage() {
	try {
	    if (inFromServer.available() > 0) {
		byte[] buffer = new byte[inFromServer.available()];
		inFromServer.read(buffer, 0, inFromServer.available());
		return new MessageFromServer(buffer[1], buffer[0], Arrays.copyOfRange(buffer, 2, buffer.length));
	    } else {
		return null;
	    }
	} catch (IOException e) {
	    // TODO Auto-generated catch block
	    e.printStackTrace();
	    isConnected = false;
	    return null;
	}
    }

    private boolean Connect() {
	// recover localhost IP address
	try {
	    iAddress = InetAddress.getLocalHost();

	    System.out.println("Trying to connect to " + iAddress.getHostAddress() + " on port " + socketPort);
	    client = new Socket(iAddress, socketPort);

	    System.out.println("Just connected to " + client.getRemoteSocketAddress());

	    outToServer = new DataOutputStream(client.getOutputStream());
	    inFromServer = new DataInputStream(client.getInputStream());
	    isConnected = true;
	    return true;
	} catch (IOException e1) {
	    // e1.printStackTrace();
	}
	isConnected = false;
	return false;
    }

    //
    // public synchronized MessageFromServer getLastMessage() {
    // if(incomingMessages.size()>0)
    // return incomingMessages.get(incomingMessages.size() - 1);
    // else
    // return null;
    // }
    //
    // public synchronized void deleteMessage(MessageFromServer message) {
    // incomingMessages.remove(message);
    // }

    public boolean Close() {
	try {
	    running = false;
	    Thread.sleep(10);

	    outToServer.close();
	    inFromServer.close();
	    client.close();
	    return true;
	} catch (IOException e) {
	    // TODO Auto-generated catch block
	    e.printStackTrace();
	} catch (InterruptedException e) {
	    // TODO Auto-generated catch block
	    e.printStackTrace();
	}
	return false;
    }

    public boolean isConnected() {
	return isConnected;
    }
}