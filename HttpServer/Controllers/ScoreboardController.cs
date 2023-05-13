using System.ComponentModel.DataAnnotations;
using System.Threading.Tasks;
using System.Text.Json;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using INF714.Data;
using INF714.Data.Providers.Interfaces;

namespace INF714.Controllers
{
    [Route("api/users/{userId}/[controller]")]
    [ApiController]
    public class ScoreboardController : ControllerBase
    {
        private IUserProvider _userProvider;

        public ScoreboardController(IUserProvider userProvider)
        {
            _userProvider = userProvider;
        }

        [Authorize("CanAccessSelfInfo")]
        [HttpGet]
        public async Task<string> GetScoreboard(string userId) {
            var user = await _userProvider.Get(userId);
            if (user == null) return "";
            
            JsonSerializerOptions policy = new JsonSerializerOptions();
            policy.PropertyNameCaseInsensitive = true;

            return JsonSerializer.Serialize<UserScoreboard>(user.Scoreboard, policy);
        }

        [Authorize("CanAccessSelfInfo")]
        [HttpPost("increase")]
        public async Task<ActionResult> IncreaseStatistics(string userId, [FromForm] [Required] string scoreboardSerialized)
        {
            var user = await _userProvider.Get(userId);
            if (user == null) return NotFound();

            JsonSerializerOptions policy = new JsonSerializerOptions();
            policy.PropertyNameCaseInsensitive = true;

            UserScoreboard scoreboard = JsonSerializer.Deserialize<UserScoreboard>(scoreboardSerialized, policy);
            user.Scoreboard += scoreboard;

            await _userProvider.Save(user);
            return Ok(user);
        }
    }
}
