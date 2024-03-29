package com.games.starwars.view.textures.obstacles;

import com.games.starwars.model.obstacles.Obstacle;
import com.games.starwars.view.textures.TexturePack;
import com.games.starwars.view.textures.TextureImpl;
import javafx.scene.layout.Pane;

public class FragileBlockTexture extends TextureImpl implements ObstacleTexture {

    public FragileBlockTexture() {
        super(0, 0, 0, 0);
        getTexture().setFill(TexturePack.imgBlackBrickPattern);
    }

    public FragileBlockTexture(double x, double y, double width, double height) {
        super(x, y, width, height);
        getTexture().setFill(TexturePack.imgBlackBrickPattern);
    }

    @Override
    public void appear(Pane pane) {
        pane.getChildren().add(getTexture());
    }

    @Override
    public void updateView(Pane pane) {

    }

    @Override
    public void removeFrom(Pane pane) {
        pane.getChildren().remove(getTexture());
    }

    @Override
    public void setObstacle(Obstacle o) {
        getTexture().setX(o.getX());
        getTexture().setY(o.getY());
        getTexture().setWidth(o.getWidth());
        getTexture().setHeight(o.getHeight());
        getTexture().setFill(TexturePack.imgBlackBrickPattern);
    }
}
