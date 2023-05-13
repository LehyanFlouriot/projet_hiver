using System;
using System.Collections.Generic;

namespace INF714.Data
{
    public class User
    {
        public string Id { get; set; }
        public string GuestToken { get; set; }

        string _name = string.Empty;
        public string Name 
        { 
            get { return _name; }
            set
            {
                if(value.Length > 16)
                {
                    throw new ArgumentOutOfRangeException("Name", "Name length must be less than 16 characters long.");
                }
                _name = value;
            }
        }
        
        public Dictionary<string, Save> Saves { get; } = new Dictionary<string, Save>();

        public UserScoreboard Scoreboard { get; set; } = new UserScoreboard();
    }
}
