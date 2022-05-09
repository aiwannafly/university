package torrent;

import be.christophedetroyer.torrent.Torrent;

import java.io.*;
import java.net.Socket;
import java.util.List;

public class LeechCommunicator implements Runnable {
    private PrintWriter out;
    private BufferedReader in;
    private final Socket leechSocket;
    private final Torrent torrent;
    private FileOutputStream fileStream;

    public LeechCommunicator(Socket leechSocket, Torrent torrentFile) {
        this.leechSocket = leechSocket;
        this.torrent = torrentFile;
        try {
            this.out = new PrintWriter(leechSocket.getOutputStream(), true);
            this.in = new BufferedReader(new InputStreamReader(leechSocket.getInputStream()));
            File receivedFile = new File(Settings.PATH + torrentFile.getName() + ".txt");
            this.fileStream = new FileOutputStream(receivedFile);
        } catch (IOException exception) {
            exception.printStackTrace();
        }
    }

    private void requestPiece(int index, int begin, int length) {
        String message = BinaryOperations.convertIntoBytes(13) + "6" +
                BinaryOperations.convertIntoBytes(index) + BinaryOperations.convertIntoBytes(begin) +
                BinaryOperations.convertIntoBytes(length);
        out.print(message);
        out.flush();
    }

    private boolean receivePiece() {
        System.out.println("Start to receive");
        StringBuilder messageBuilder = new StringBuilder();
        try {
            for (int i = 0; i < 4; i++) {
                messageBuilder.append((char) in.read());
            }
            int messageLength = BinaryOperations.convertFromBytes(messageBuilder.toString());
            // System.out.println("Message length: " + messageLength);
            for (int i = 0; i < messageLength; i++) {
                messageBuilder.append((char) in.read());
            }
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        String message = messageBuilder.toString();
        if (message.length() < 4 + 1 + 4 + 4) {
            return false;
        }
        // piece: <len=0009+X><id=7><index><begin><block>
        int len = BinaryOperations.convertFromBytes(message.substring(0, 4));
        int id = Integer.parseInt(String.valueOf(message.charAt(4)));
        // System.out.println("len: " + len);
        // System.out.println("id: " + id);
        if (id != Settings.PIECE_ID) {
            return false;
        }
        int idx = BinaryOperations.convertFromBytes(message.substring(5, 9));
        int begin = BinaryOperations.convertFromBytes(message.substring(9, 13));
        try {
            String data = message.substring(13);
            System.out.println("DATA: " + data);
            byte[] bytes = BinaryOperations.getBytesFromString(data);
            for (int i = 0; i < len - 9; i++) {
                System.out.println((int) data.charAt(i));
            }
            fileStream.write(bytes);
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    @Override
    public void run() {
        System.out.println("Start requesting...");
        List<String> pieces = torrent.getPieces();
        for (int i = 0; i < pieces.size(); i++) {
            int pieceLength;
            if (i == pieces.size() - 1) {
                pieceLength = torrent.getTotalSize().intValue() % torrent.getPieceLength().intValue();
            } else {
                pieceLength = torrent.getPieceLength().intValue();
            }
            requestPiece(i, 0, pieceLength);
            System.out.println("Requested");
            boolean received = receivePiece();
            System.out.println("Received");
            if (!received) {
                System.out.println("Failed to receive a piece");
            }
        }
        System.out.println("Torrent file was downloaded successfully!");
        try {
            if (out != null) {
                out.close();
            }
            if (in != null) {
                in.close();
            }
            leechSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
