package com.RTR.RoboticArm;

public class Stack
{
    private static int MAX = 10;
    private float[][] matrix_stack;
    private int top;

    public Stack()
    {
        matrix_stack = new float[MAX][16];
        top = -1;
    }

    public boolean push(float[] matrix)
    {
        if(top > (MAX - 1))
        {
            return (false);
        }
        else
        {
            top++;
            matrix_stack[top] = matrix;
            return (true);
        }
    }

    public float[] pop()
    {
        if(top < 0)
        {
            return (null);
        }
        else
        {
            float[] matrix = new float[16];
            matrix = matrix_stack[top];
            top--;
            return (matrix);
        }
    }
}