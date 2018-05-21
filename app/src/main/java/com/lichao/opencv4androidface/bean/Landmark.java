package com.lichao.opencv4androidface.bean;

/**
 * Created by ChaoLi on 2018/5/20 0020 - 22:36
 * Email: lichao3140@gmail.com
 * Version: v1.0
 */
public class Landmark {

    private int x;

    private int y;

    public int getX() {
        return x;
    }

    public void setX(int x) {
        this.x = x;
    }

    public int getY() {
        return y;
    }

    public void setY(int y) {
        this.y = y;
    }

    @Override
    public String toString() {
        return "Landmark{" +
                "x=" + x +
                ", y=" + y +
                '}';
    }

}
