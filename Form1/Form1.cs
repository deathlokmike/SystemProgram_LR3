﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;

namespace Form1
{
    public partial class Form1 : Form
    {
        [DllImport("MFCLibrary.dll", CharSet = CharSet.Ansi)] private static extern void Send(int actionCode, string message, int threadNumber = -1/*значение по умолчанию, чтобы при отправки только кода действия каждый раз не прописывать номер потока, так как в этом случае он не нужен*/);
        [DllImport("MFCLibrary.dll")] public static extern void waitEventConfirm();
        [DllImport("MFCLibrary.dll")] public static extern void Init();
        [DllImport("MFCLibrary.dll")] public static extern bool ProcessisOpen();
        public Form1()
        {
            InitializeComponent();
        }

        private bool console()
        {
            if (!ProcessisOpen())
            {
                treeView1.Nodes.Clear();
                return false;
            }
            return true;
        }
        private void add_child()
        {
            if (treeView1.Nodes.Count == 1)
            {
                treeView1.Nodes.Add("Все потоки");
            }
            treeView1.Nodes[1].Nodes.Add("Поток " + (treeView1.Nodes[1].Nodes.Count + 1).ToString());
        }

        private void treeView1_AfterCheck(object sender, TreeViewEventArgs e)
        {
            foreach (TreeNode child in e.Node.Nodes)
            {
                child.Checked = e.Node.Checked;
            }
        }
        
        private List<int> get_current_num() // выдает значения по выбранным потоком
        {
            List<int> buff = new List<int>();
            if (treeView1.Nodes.Count > 0)
            {
                if (treeView1.Nodes[0].Checked)
                    buff.Add(-2);
                if (treeView1.Nodes.Count > 1)
                {
                    if (treeView1.Nodes[1].Checked)
                        buff.Add(-1);
                    else
                    {
                        int j = 0;
                        foreach (TreeNode child in treeView1.Nodes[1].Nodes)
                        {
                            if (child.Checked)
                                buff.Add(j);
                            j += 1;
                        }
                    }    
                }
            }
            return buff;
        }
        private void button1_Click(object sender, EventArgs e)
        {
            if (console())
            {
                int thread_count = Convert.ToInt32(textBox1.Text);
                for (int j = 0; j < thread_count; j++)
                {
                    Send(0, "");//отправка кода действия 
                    waitEventConfirm();
                    add_child();
                }
            }
            else
            {
                treeView1.Nodes.Add("Главный поток");
                Init();//запуск консоли при помощи dll
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (console())
            {
                Send(1, "");
                if (treeView1.Nodes.Count != 1)
                {
                    if (treeView1.Nodes[1].Nodes.Count > 1)
                        treeView1.Nodes[1].Nodes.RemoveAt(treeView1.Nodes[1].Nodes.Count - 1);
                    else
                        treeView1.Nodes.RemoveAt(1);
                }
                else
                {
                    treeView1.Nodes.RemoveAt(0);
                }
                waitEventConfirm();
            }
            else
            {
                Send(2, "");
                waitEventConfirm();
                //Child = null;
                return;
            }
        }
        private void CloseApp(object sender, FormClosingEventArgs e)
        {
            if (console())
            {
                Send(2, "");
                waitEventConfirm();
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            string text = textBox2.Text;
            List<int> thread_nums = get_current_num();
            if (text.Length == 0 || thread_nums.Count == 0) return;
            foreach (int threadNumber in thread_nums)
            {
                Send(3, text, threadNumber);//отправка сообщения вместе с кодом действия отправки сообщения
            }
        }
    }
}
