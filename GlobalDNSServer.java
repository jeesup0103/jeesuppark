import java.io.*;
import java.net.*;

public class GlobalDNSServer {

    public static String lookup(String domain) {
        String[] files = {"global_edu_dns.txt", "global_com_dns.txt"};
        for (String fileName : files) {
            try (BufferedReader br = new BufferedReader(new FileReader(fileName))) {
                String line;
                while ((line = br.readLine()) != null) {
                    String[] parts = line.trim().split("\\s+");
                    if (parts.length == 2 && parts[0].equals(domain)) {
                        return parts[1];
                    }
                }
            } catch (IOException e) {
                System.err.println("Error reading " + fileName);
            }
        }
        return "Not Found";
    }

    public static void main(String[] args) {
        final int PORT = 9000;
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            System.out.println("Global DNS listening on TCP " + PORT + "...");
            while (true) {
                try (
                    Socket clientSocket = serverSocket.accept();
                    BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                    BufferedWriter out = new BufferedWriter(new OutputStreamWriter(clientSocket.getOutputStream()))
                ) {
                    String domain = in.readLine();
                    String response = lookup(domain.trim());
                    out.write(response);
                    out.newLine();
                    out.flush();
                } catch (IOException e) {
                    System.err.println("Client connection error: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("Could not listen on port " + PORT);
            e.printStackTrace();
        }
    }
}
