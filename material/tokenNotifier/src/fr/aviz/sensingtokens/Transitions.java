package fr.aviz.sensingtokens;

public class Transitions {

    public final static byte STILL			= 0x1;
    public final static byte STILL_CONTACT		= 0x2;
    public final static byte MOVING 			= 0x4;
    public final static byte MOVING_CONTACT  		= 0x3;

    public final static byte NO_TRANSITION   		= 0x0;
    public final static byte FALL 			= transition(STILL, MOVING);
    public final static byte TOUCH 			= transition(STILL, STILL_CONTACT);
    public final static byte STILL_RELEASE 		= transition(STILL_CONTACT, STILL);
    public final static byte STILL_GRIP_CHANGED 	= transition(STILL_CONTACT, STILL_CONTACT);
    public final static byte MOVE 			= transition(STILL_CONTACT, MOVING_CONTACT);
    public final static byte STOP 			= transition(MOVING_CONTACT, STILL_CONTACT);
    public final static byte MOVING_GRIP_CHANGED	= transition(MOVING_CONTACT, MOVING_CONTACT);
    public final static byte THROW 			= transition(MOVING_CONTACT, MOVING);
    public final static byte HALT 			= transition(MOVING, STILL);
    public final static byte HIT_CATCH 			= transition(MOVING, MOVING_CONTACT);

    public final static byte transition(byte initialState, byte finalState) {
	return (byte) (initialState << 4 | finalState);
    }

}
