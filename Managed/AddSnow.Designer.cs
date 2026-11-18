
namespace Spritesheet2Gif
{
    partial class AddSnow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.chooseColor = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.button3 = new System.Windows.Forms.Button();
            this.yVelocity = new System.Windows.Forms.NumericUpDown();
            this.columnSpacingBase = new System.Windows.Forms.NumericUpDown();
            this.columnSpacingRandomIncrementLimit = new System.Windows.Forms.NumericUpDown();
            this.columnInitialYPositionBase = new System.Windows.Forms.NumericUpDown();
            this.columnInitialYPositionRandomIncrementLimit = new System.Windows.Forms.NumericUpDown();
            this.oneOverXVelocityBase = new System.Windows.Forms.NumericUpDown();
            this.oneOverXVelocityRandomIncrementLimit = new System.Windows.Forms.NumericUpDown();
            ((System.ComponentModel.ISupportInitialize)(this.yVelocity)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.columnSpacingBase)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.columnSpacingRandomIncrementLimit)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.columnInitialYPositionBase)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.columnInitialYPositionRandomIncrementLimit)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.oneOverXVelocityBase)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.oneOverXVelocityRandomIncrementLimit)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 20);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(64, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Snow Color:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(47, 67);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(114, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Column Spacing Base:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(47, 97);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(204, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "Column Spacing Random Increment Limit:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(47, 130);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(149, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "Column Initial Y Position Base:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(47, 162);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(239, 13);
            this.label5.TabIndex = 4;
            this.label5.Text = "Column Initial Y Position Random Increment Limit:";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(47, 194);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(133, 13);
            this.label6.TabIndex = 5;
            this.label6.Text = "One Over X Velocity Base:";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(47, 226);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(223, 13);
            this.label7.TabIndex = 6;
            this.label7.Text = "One Over X Velocity Random Increment Limit:";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(47, 261);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(57, 13);
            this.label8.TabIndex = 7;
            this.label8.Text = "Y Velocity:";
            // 
            // chooseColor
            // 
            this.chooseColor.Location = new System.Drawing.Point(92, 11);
            this.chooseColor.Name = "chooseColor";
            this.chooseColor.Size = new System.Drawing.Size(115, 30);
            this.chooseColor.TabIndex = 8;
            this.chooseColor.Text = "Choose Color";
            this.chooseColor.UseVisualStyleBackColor = true;
            this.chooseColor.Click += new System.EventHandler(this.chooseColor_Click);
            // 
            // button2
            // 
            this.button2.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.button2.Location = new System.Drawing.Point(16, 310);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(115, 30);
            this.button2.TabIndex = 9;
            this.button2.Text = "Generate";
            this.button2.UseVisualStyleBackColor = true;
            // 
            // button3
            // 
            this.button3.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.button3.Location = new System.Drawing.Point(246, 310);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(115, 30);
            this.button3.TabIndex = 10;
            this.button3.Text = "Cancel";
            this.button3.UseVisualStyleBackColor = true;
            // 
            // yVelocity
            // 
            this.yVelocity.Location = new System.Drawing.Point(292, 259);
            this.yVelocity.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.yVelocity.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.yVelocity.Name = "yVelocity";
            this.yVelocity.Size = new System.Drawing.Size(69, 20);
            this.yVelocity.TabIndex = 11;
            this.yVelocity.Value = new decimal(new int[] {
            3,
            0,
            0,
            0});
            // 
            // columnSpacingBase
            // 
            this.columnSpacingBase.Location = new System.Drawing.Point(292, 65);
            this.columnSpacingBase.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.columnSpacingBase.Name = "columnSpacingBase";
            this.columnSpacingBase.Size = new System.Drawing.Size(69, 20);
            this.columnSpacingBase.TabIndex = 12;
            this.columnSpacingBase.Value = new decimal(new int[] {
            12,
            0,
            0,
            0});
            // 
            // columnSpacingRandomIncrementLimit
            // 
            this.columnSpacingRandomIncrementLimit.Location = new System.Drawing.Point(292, 95);
            this.columnSpacingRandomIncrementLimit.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.columnSpacingRandomIncrementLimit.Name = "columnSpacingRandomIncrementLimit";
            this.columnSpacingRandomIncrementLimit.Size = new System.Drawing.Size(69, 20);
            this.columnSpacingRandomIncrementLimit.TabIndex = 13;
            this.columnSpacingRandomIncrementLimit.Value = new decimal(new int[] {
            4,
            0,
            0,
            0});
            // 
            // columnInitialYPositionBase
            // 
            this.columnInitialYPositionBase.Location = new System.Drawing.Point(292, 128);
            this.columnInitialYPositionBase.Maximum = new decimal(new int[] {
            200,
            0,
            0,
            0});
            this.columnInitialYPositionBase.Minimum = new decimal(new int[] {
            200,
            0,
            0,
            -2147483648});
            this.columnInitialYPositionBase.Name = "columnInitialYPositionBase";
            this.columnInitialYPositionBase.Size = new System.Drawing.Size(69, 20);
            this.columnInitialYPositionBase.TabIndex = 14;
            this.columnInitialYPositionBase.Value = new decimal(new int[] {
            80,
            0,
            0,
            -2147483648});
            // 
            // columnInitialYPositionRandomIncrementLimit
            // 
            this.columnInitialYPositionRandomIncrementLimit.Location = new System.Drawing.Point(292, 160);
            this.columnInitialYPositionRandomIncrementLimit.Maximum = new decimal(new int[] {
            200,
            0,
            0,
            0});
            this.columnInitialYPositionRandomIncrementLimit.Name = "columnInitialYPositionRandomIncrementLimit";
            this.columnInitialYPositionRandomIncrementLimit.Size = new System.Drawing.Size(69, 20);
            this.columnInitialYPositionRandomIncrementLimit.TabIndex = 15;
            this.columnInitialYPositionRandomIncrementLimit.Value = new decimal(new int[] {
            80,
            0,
            0,
            0});
            // 
            // oneOverXVelocityBase
            // 
            this.oneOverXVelocityBase.Location = new System.Drawing.Point(292, 192);
            this.oneOverXVelocityBase.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.oneOverXVelocityBase.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.oneOverXVelocityBase.Name = "oneOverXVelocityBase";
            this.oneOverXVelocityBase.Size = new System.Drawing.Size(69, 20);
            this.oneOverXVelocityBase.TabIndex = 16;
            this.oneOverXVelocityBase.Value = new decimal(new int[] {
            4,
            0,
            0,
            0});
            // 
            // oneOverXVelocityRandomIncrementLimit
            // 
            this.oneOverXVelocityRandomIncrementLimit.Location = new System.Drawing.Point(292, 226);
            this.oneOverXVelocityRandomIncrementLimit.Maximum = new decimal(new int[] {
            20,
            0,
            0,
            0});
            this.oneOverXVelocityRandomIncrementLimit.Name = "oneOverXVelocityRandomIncrementLimit";
            this.oneOverXVelocityRandomIncrementLimit.Size = new System.Drawing.Size(69, 20);
            this.oneOverXVelocityRandomIncrementLimit.TabIndex = 17;
            this.oneOverXVelocityRandomIncrementLimit.Value = new decimal(new int[] {
            10,
            0,
            0,
            0});
            // 
            // AddSnow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(400, 369);
            this.Controls.Add(this.oneOverXVelocityRandomIncrementLimit);
            this.Controls.Add(this.oneOverXVelocityBase);
            this.Controls.Add(this.columnInitialYPositionRandomIncrementLimit);
            this.Controls.Add(this.columnInitialYPositionBase);
            this.Controls.Add(this.columnSpacingRandomIncrementLimit);
            this.Controls.Add(this.columnSpacingBase);
            this.Controls.Add(this.yVelocity);
            this.Controls.Add(this.button3);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.chooseColor);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AddSnow";
            this.Text = "Add Snow";
            ((System.ComponentModel.ISupportInitialize)(this.yVelocity)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.columnSpacingBase)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.columnSpacingRandomIncrementLimit)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.columnInitialYPositionBase)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.columnInitialYPositionRandomIncrementLimit)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.oneOverXVelocityBase)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.oneOverXVelocityRandomIncrementLimit)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Button chooseColor;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.NumericUpDown yVelocity;
        private System.Windows.Forms.NumericUpDown columnSpacingBase;
        private System.Windows.Forms.NumericUpDown columnSpacingRandomIncrementLimit;
        private System.Windows.Forms.NumericUpDown columnInitialYPositionBase;
        private System.Windows.Forms.NumericUpDown columnInitialYPositionRandomIncrementLimit;
        private System.Windows.Forms.NumericUpDown oneOverXVelocityBase;
        private System.Windows.Forms.NumericUpDown oneOverXVelocityRandomIncrementLimit;
    }
}