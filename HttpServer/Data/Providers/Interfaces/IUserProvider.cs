using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using INF714.Data;

namespace INF714.Data.Providers.Interfaces
{
    public interface IUserProvider
    {
        Task Create(User user);
        Task<User> Get(string id);
        Task Save(User user);
    }
}
