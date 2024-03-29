package com.games.starwars.FXML;

import com.games.starwars.Settings;
import com.games.starwars.controller.Runner;
import com.games.starwars.controller.RunnerImpl;
import com.games.starwars.view.SoundsPlayer;
import com.games.starwars.view.SceneBuilder;
import javafx.fxml.FXML;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.stage.Stage;

public class FXMLLevels {

    @FXML
    private Button backButton;

    private void setLevel(int level) {
        Stage stage = (Stage) backButton.getScene().getWindow();
        Runner runner = SceneBuilder.getGameRunner(stage, level);
        runner.run();
    }

    @FXML
    protected void onLevelOneClick() {
        if (Settings.soundsON) {
            SoundsPlayer.playButtonClick();
        }
        setLevel(1);
    }

    @FXML
    protected void onLevelTwoClick() {
        if (Settings.soundsON) {
            SoundsPlayer.playButtonClick();
        }
        setLevel(2);
    }

    @FXML
    protected void onLevelThreeClick() {
        if (Settings.soundsON) {
            SoundsPlayer.playButtonClick();
        }
        setLevel(3);
    }

    @FXML
    protected void onLevelFourClick() {
        if (Settings.soundsON) {
            SoundsPlayer.playButtonClick();
        }
        setLevel(4);
    }

    @FXML
    protected void onBackClick() {
        if (Settings.soundsON) {
            SoundsPlayer.playButtonClick();
        }
        Stage stage = (Stage) backButton.getScene().getWindow();
        Scene scene = SceneBuilder.getMenuScene();
        stage.setScene(scene);
    }
}
