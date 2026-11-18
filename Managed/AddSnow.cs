using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Spritesheet2Gif
{
    public partial class AddSnow : Form
    {
        public AddSnow()
        {
            InitializeComponent();

            chooseColor.BackColor = System.Drawing.Color.White;
        }

        private void chooseColor_Click(object sender, EventArgs e)
        {
            using (ColorDialog colorDialog = new ColorDialog())
            {
                colorDialog.AllowFullOpen = true;
                colorDialog.Color = chooseColor.BackColor;

                if (colorDialog.ShowDialog() == DialogResult.OK)
                {
                    chooseColor.BackColor = colorDialog.Color;
                }

            }
        }

        public int GetColorArgb()
        {
            return chooseColor.BackColor.ToArgb();
        }

        public int GetColumnSpacingBase()
        {
            return (int)columnSpacingBase.Value;
        }

        public int GetColumnSpacingRandomIncrementLimit()
        {
            return (int)columnSpacingRandomIncrementLimit.Value;
        }

        public int GetColumnInitialYPositionBase()
        {
            return (int)columnInitialYPositionBase.Value;
        }

        public int GetColumnInitialYPositionRandomIncrementLimit()
        {
            return (int)columnInitialYPositionRandomIncrementLimit.Value;
        }

        public int GetOneOverXVelocityBase()
        {
            return (int)oneOverXVelocityBase.Value;
        }

        public int GetOneOverXVelocityRandomIncrementLimit()
        {
            return (int)oneOverXVelocityRandomIncrementLimit.Value;
        }

        public int GetYVelocity()
        {
            return (int)yVelocity.Value;
        }
    }
}
