using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using System.Diagnostics;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Table;
using Microsoft.WindowsAzure.Storage.Auth;
using System.Xml.Serialization;



namespace T_Glove
{
    class Finger_Flex : Object
    {
        private const int MAX_HISTORY_LEN = 100;
        public const int INVALID_GOODNESS = -1;
        public const int RESET_THRESHOLD = 5;
        public const int RESET_RANGE = 3;
        int[] history = new int[MAX_HISTORY_LEN];
        int next_index = 0;
        private const int target_val = 100;
        int local_max = INVALID_GOODNESS;
        /*
         * check if we 
         */
        private int check_reset()
        {
            int max = local_max;
            if (next_index > 0 && max >= RESET_THRESHOLD && history[next_index - 1] <= RESET_RANGE)
            {
                next_index = 0;
                local_max = INVALID_GOODNESS;
                return max;
            }
            return INVALID_GOODNESS;
        }

        public int insertVal(int val)
        {
            if (0 == next_index || history[next_index - 1] != val)
            {
                history[next_index] = val;
                next_index++;
                //condition? first_expression : second_expression;
                local_max = (local_max >= val) ? local_max : val;
                if (next_index == MAX_HISTORY_LEN)
                {
                    next_index = 0;
                }
               
            }
            return check_reset();
        }

        public int get_current_goodness()
        {
            return local_max;
        }



    }
    public class HandRoll : Object
    {
        private const int MAX_HISTORY_LEN = 300;
        private int[] history = new int[MAX_HISTORY_LEN];
        private const int target_val = 100;
        private const int RESET_ABS_THRESHOLD = 5;
        private const int RESET_ABS_RANGE = 3;
        public const int INVALID_GOODNESS = -1;
        int next_index = 0;


        private int check_reset()
        {
            int max = Math.Abs(history.Max());
            if (next_index > 0 && max >= RESET_ABS_THRESHOLD && Math.Abs(history[next_index - 1]) <= RESET_ABS_RANGE)
            {
                next_index = 0;
                Array.Clear(history, 0, history.Length);
                return max;
            }
            return INVALID_GOODNESS;
        }
        
        public int insertVal(int val)
        {
            if (0 == next_index || history[next_index - 1] != val)
            {
                history[next_index] = val;
                next_index++;
                if (next_index == MAX_HISTORY_LEN)
                {
                    next_index = 0;
                }

            }
            return check_reset();
        }

    }
    public class Session : TableEntity
    {

        public enum MOVMENT_MODE { ROLL, FIST };

        public class ChartParams
        {
            public string Name { get; set; }
            public int Amount { get; set; }
        }

        // BASEL debug : sorry had to turn this to public TODO tuen back to private
        public List<int> finger_flex_goodness = new List<int>();
        public string finget_flex_goodness_serialized
        {
            get;
            set;
        }

        public List<int> hand_roll_goodness = new List<int>();
        public string hand_roll_goodness_serialized
        {
            get;
            set;
        }
        private Finger_Flex finger_flex_controler = new Finger_Flex();
        private HandRoll hand_roll_controller = new HandRoll();
        public bool SESSION_ALIVE = false;

        public Session()
        {
            this.PartitionKey = "default";
           // this.RowKey = (App.SessionIdCounter++).ToString();
        }

        public Session copy_session()
        {
            Session new_session = new Session();
            //new_session.finger_flex_goodness = new List<int>();
            //new_session.hand_roll_goodness = new List<int>();
            new_session.finger_flex_goodness = this.finger_flex_goodness;
            new_session.hand_roll_goodness = this.hand_roll_goodness;

            /*foreach (int i in this.finger_flex_goodness)
            {
                int num = new int();
                num = i;
                new_session.finger_flex_goodness.Add(num);
            }
            foreach (int i in this.hand_roll_goodness)
            {
                int num = new int();
                num = i;
                new_session.hand_roll_goodness.Add(num);
            }*/
            return new_session;
        }
        // new roll class
        public void record_raw_vals(int rol, int potentiometer)
        {
            if (!SESSION_ALIVE)
            {
                return; // don't record until the session starts
            }

            int fist_goodness = finger_flex_controler.insertVal(Math.Abs(potentiometer));
            int roll_goodness = hand_roll_controller.insertVal(Math.Abs(rol));
            if (fist_goodness != Finger_Flex.INVALID_GOODNESS) // There is  reset
            {
                finger_flex_goodness.Add(fist_goodness);
            }
            if (roll_goodness != Finger_Flex.INVALID_GOODNESS) // There is  reset
            {
                hand_roll_goodness.Add(roll_goodness);
            }
        }

        // BASEL
        public int GetGoodnessAverage(MOVMENT_MODE mode)
        {
            List<int> goodness_list;
            if (MOVMENT_MODE.ROLL == mode)
            {
                goodness_list = hand_roll_goodness;
            }
            else
            {
                goodness_list = finger_flex_goodness;
            }
            int size = goodness_list.Count;
            if(size == 0)
            {
                return 0;
            }
            int sum = 0;
            foreach (int goodness in goodness_list)
            {
                sum += goodness;
            }
            return sum /size;
        }


        public List<ChartParams> build_chart(MOVMENT_MODE mode)
        {
            List<int> goodness_list;
            if (MOVMENT_MODE.ROLL==mode)
            {
                goodness_list = hand_roll_goodness;
            }
            else
            {
                goodness_list = finger_flex_goodness;
            }
            List<ChartParams> chartVals = new List<ChartParams>();
            int i = 1;
            foreach (int movement in goodness_list)
            {
                chartVals.Add(new ChartParams() { Name = "mov" + i.ToString(), Amount = movement });
                i++;
            }
            //chartVals.Add(new ChartParams() { Name = "mov" + i.ToString(), Amount = finger_flex_controler.get_current_goodness()});
            //i++;
            return chartVals;
        }
    }
    
}
