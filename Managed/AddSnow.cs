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
        public AddSnow(Settings s)
        {
            InitializeComponent();

            chooseColor.BackColor = s.ColorArgb;
            particleSize.Value = s.ParticleSize;
            initialXPosition.Value = s.InitialXPosition;
            columnSpacingBase.Value = s.ColumnSpacingBase;
            columnSpacingRandomIncrementLimit.Value = s.ColumnSpacingRandomIncrementLimit;
            columnInitialYPositionBase.Value = s.ColumnInitialYPositionBase;
            columnInitialYPositionRandomIncrementLimit.Value = s.ColumnInitialYPositionRandomIncrementLimit;
            oneOverXVelocityBase.Value = s.OneOverXVelocityBase;
            oneOverXVelocityRandomIncrementLimit.Value = s.OneOverXVelocityRandomIncrementLimit;
            yVelocity.Value = s.YVelocity;
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

        public struct Settings
        {
            public Color ColorArgb;
            public decimal ParticleSize;
            public decimal InitialXPosition;
            public decimal ColumnSpacingBase;
            public decimal ColumnSpacingRandomIncrementLimit;
            public decimal ColumnInitialYPositionBase;
            public decimal ColumnInitialYPositionRandomIncrementLimit;
            public decimal OneOverXVelocityBase;
            public decimal OneOverXVelocityRandomIncrementLimit;
            public decimal YVelocity;
        }

        public Settings GetSettings()
        {
            return new Settings() 
            { 
                ColorArgb = chooseColor.BackColor,
                ParticleSize = particleSize.Value,
                InitialXPosition = initialXPosition.Value,
                ColumnSpacingBase = columnSpacingBase.Value,
                ColumnSpacingRandomIncrementLimit = columnSpacingRandomIncrementLimit.Value,
                ColumnInitialYPositionBase = columnInitialYPositionBase.Value,
                ColumnInitialYPositionRandomIncrementLimit = columnSpacingRandomIncrementLimit.Value,
                OneOverXVelocityBase = oneOverXVelocityBase.Value,
                OneOverXVelocityRandomIncrementLimit = oneOverXVelocityRandomIncrementLimit.Value,
                YVelocity = yVelocity.Value
            };
        }

    }
}
