using System;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace Spritesheet2Gif
{
    public partial class Form1 : Form
    {
        Graphics canvasGraphics;
        IntPtr canvasHdc;
        bool autoplay;

        public Form1()
        {
            InitializeComponent();

            canvasGraphics = canvas.CreateGraphics();
            canvasHdc = canvasGraphics.GetHdc();
            if (!Native.Initialize(canvas.Width, canvas.Height, canvasHdc))
            {
                this.Load += ErrorOnLoad;
            }

            autoplay = false;
        }

        private void OpenToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Native.OpenSpritesheetFile(this.Handle);

            int w = Native.GetSpritesheetWidth();
            if (w == 0)
                return;

            int h = Native.GetSpritesheetHeight();
            if (h == 0)
                return;

            spriteWidth.Minimum = 1;
            spriteWidth.Maximum = w;
            spriteWidth.Value = w;
            spriteWidth.Enabled = true;

            spriteHeight.Minimum = 1;
            spriteHeight.Maximum = h;
            spriteHeight.Value = h;
            spriteHeight.Enabled = true;

            previousBtn.Enabled = true;
            nextBtn.Enabled = true;
            autoplayBtn.Enabled = true;
            gifSpeed.Enabled = true;

            gifSpeed.Minimum = 10;
            gifSpeed.Maximum = 5000;
            gifSpeed.Value = 100;
            gifSpeed.Increment = 10;

            Native.Paint();
        }

        private void ExitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        void ErrorOnLoad(object sender, EventArgs e)
        {
            MessageBox.Show("An error occurred while initializing Direct2D.", "Error");

            Close();
        }

        private void Canvas_Paint(object sender, PaintEventArgs e)
        {
            Native.Paint();
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            Native.Uninitialize();
        }

        private void Form1_SizeChanged(object sender, EventArgs e)
        {
            Native.OnResize(canvas.Width, canvas.Height, canvasHdc);
        }

        private void SpriteWidth_ValueChanged(object sender, EventArgs e)
        {
            Native.SetSpriteWidth((int)spriteWidth.Value);
            Native.Paint();
        }

        private void SpriteHeight_ValueChanged(object sender, EventArgs e)
        {
            Native.SetSpriteHeight((int)spriteHeight.Value);
            Native.Paint();
        }

        private void Form1_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Left)
            {
                Native.PreviousSprite();
                Native.Paint();
            }
            else if (e.KeyCode == Keys.Right)
            {
                Native.NextSprite();
                Native.Paint();
            }
        }

        void ForceStopAutoplay()
        {
            if (autoplay)
            {
                autoplayBtn.Text = "Autoplay: off";
                Native.SetAutoplay(canvas.Handle, 0);
                autoplay = false;
            }
        }

        private void PreviousBtn_Click(object sender, EventArgs e)
        {
            ForceStopAutoplay();
            Native.PreviousSprite();
            Native.Paint();
        }

        private void NextBtn_Click(object sender, EventArgs e)
        {
            ForceStopAutoplay();
            Native.NextSprite();
            Native.Paint();
        }

        private void AutoplayBtn_Click(object sender, EventArgs e)
        {
            autoplay = !autoplay;
            if (autoplay)
            {
                autoplayBtn.Text = "Autoplay: on";
            }
            else
            {
                autoplayBtn.Text = "Autoplay: off";
            }

            Native.SetAutoplay(canvas.Handle, autoplay ? 1 : 0);
        }

        private void AutoplaySpeed_ValueChanged(object sender, EventArgs e)
        {
            Native.SetAutoplaySpeed(canvas.Handle, (int)gifSpeed.Value);
        }

        private void SaveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Native.SaveGif(this.Handle, (int)gifSpeed.Value);
        }

        private void ZoomInToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Native.ZoomIn();
            Native.Paint();
        }

        private void ZoomOutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Native.ZoomOut();
            Native.Paint();
        }

        private void ResetZoomToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Native.ResetZoom();
            Native.Paint();
        }
    }
}

class Native
{
    [DllImport("Native.dll")]
    public static extern bool Initialize(int clientWidth, int clientHeight, IntPtr handle);
    [DllImport("Native.dll")]
    public static extern void OnResize(int clientWidth, int clientHeight, IntPtr handle);

    [DllImport("Native.dll")]
    public static extern void Paint();

    [DllImport("Native.dll")]
    public static extern void OpenSpritesheetFile(IntPtr parentDialog);
    [DllImport("Native.dll")]
    public static extern int GetSpritesheetWidth();
    [DllImport("Native.dll")]
    public static extern int GetSpritesheetHeight();
    [DllImport("Native.dll")]
    public static extern void SetSpriteWidth(int w);
    [DllImport("Native.dll")]
    public static extern void SetSpriteHeight(int h);

    [DllImport("Native.dll")]
    public static extern void PreviousSprite();

    [DllImport("Native.dll")]
    public static extern void NextSprite();

    [DllImport("Native.dll")]
    public static extern void SetAutoplay(IntPtr parentDialog, int value);

    [DllImport("Native.dll")]
    public static extern void SetAutoplaySpeed(IntPtr parentDialog, int value);

    [DllImport("Native.dll")]
    public static extern void SaveGif(IntPtr parentDialog, int animationSpeed);

    [DllImport("Native.dll")]
    public static extern void ZoomIn();

    [DllImport("Native.dll")]
    public static extern void ZoomOut();

    [DllImport("Native.dll")]
    public static extern void ResetZoom();

    [DllImport("Native.dll")]
    public static extern void Uninitialize();
}