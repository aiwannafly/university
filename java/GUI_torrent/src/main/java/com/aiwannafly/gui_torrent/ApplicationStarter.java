package com.aiwannafly.gui_torrent;

import com.aiwannafly.gui_torrent.controller.MainMenuController;
import com.aiwannafly.gui_torrent.torrent.client.BitTorrentClient;
import com.aiwannafly.gui_torrent.torrent.client.TorrentClient;
import com.aiwannafly.gui_torrent.view.Renderer;
import javafx.application.Application;
import javafx.scene.Scene;
import javafx.stage.Stage;

import java.io.IOException;

public class ApplicationStarter extends Application {
    private static TorrentClient torrentClient;

    @Override
    public void start(Stage stage) throws IOException {
        torrentClient = new BitTorrentClient();
        Scene scene = Renderer.getScene();
        stage.setTitle("aiTorrent");
        stage.setScene(scene);
        stage.setOnCloseRequest(event -> MainMenuController.exit());
        stage.show();
    }

    public static void main(String[] args) {
        launch();
    }

    public static TorrentClient getTorrentClient() {
        return torrentClient;
    }

}