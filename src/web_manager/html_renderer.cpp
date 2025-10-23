#include "html_renderer.h"

std::string HtmlRenderer::renderSettingsPage(const std::vector<Calendar> &calendars, const std::string &primaryHex, const std::string &bgHex)
{
    std::string html;
    html += "<!doctype html><html><head><meta charset='utf-8'><title>Device Settings</title>";
    html += "<meta name=viewport content='width=device-width,initial-scale=1'/>";
    html += "<style>html,body{height:100%;margin:0;font-family:Inter,Arial,Helvetica,sans-serif;background:#f5f7fb;color:#0f172a}header{background:#fff;padding:16px 20px;border-bottom:1px solid #e6e9ef}h1{margin:0;font-size:18px;font-weight:600}main{max-width:1100px;margin:18px auto;padding:0 16px}form{display:block;gap:18px;align-items:start} .panel{background:#fff;border:1px solid #e6e9ef;border-radius:10px;padding:12px;margin-bottom:12px} .cal{display:flex;align-items:flex-start;gap:12px;padding:10px;border-bottom:1px solid #f1f5f9} .cal:last-child{border-bottom:none} .meta{flex:1;min-width:0} .title{font-weight:600;color:#0b1220;white-space:normal;overflow-wrap:anywhere;word-break:break-word} .desc{font-size:13px;color:#475569;margin-top:6px;white-space:normal;overflow-wrap:anywhere;word-break:break-word;max-height:6em;overflow:auto;padding-right:6px} .primary{font-size:12px;color:#0ea5a4;margin-top:6px;font-weight:700} .id{font-size:12px;color:#667085;margin-top:6px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;max-width:100%} .actions{display:flex;align-items:center;gap:8px} .switch{position:relative;display:inline-block;width:50px;height:28px} .switch input{display:none} .slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background:#e2e8f0;border-radius:999px;transition:all .18s} .slider:before{content:'';position:absolute;left:4px;top:4px;width:20px;height:20px;background:#fff;border-radius:50%;box-shadow:0 1px 2px rgba(2,6,23,0.08);transition:all .18s} input:checked+.slider{background:#22c55e} input:checked+.slider:before{transform:translateX(22px)} .controls{display:flex;flex-direction:column;gap:12px} label.field{display:flex;flex-direction:column;font-size:13px;color:#0f172a} input[type=color]{width:56px;height:34px;border:0;padding:0;background:transparent;border-radius:6px} .btn-row{display:flex;gap:10px;justify-content:flex-end} button{background:#2563eb;border:none;color:#fff;padding:9px 14px;border-radius:8px;cursor:pointer;font-weight:600} button.secondary{background:#f8fafc;color:#0b1220;border:1px solid #e6e9ef} @media(max-width:880px){form{padding-bottom:18px}}</style>";

    html += "</head><body>";
    html += "<header><h1>Device Settings</h1></header>";
    html += "<main>";
    html += "<form method=post action='/settings'>";

    // Calendars
    html += "<div class=panel>";
    html += "<h2 style='margin:0 0 10px 0;font-size:15px'>Calendars</h2>";
    for(size_t i = 0; i < calendars.size(); ++i)
    {
        const auto &c = calendars[i];
        html += "<div class=cal>";
        html += "<div class=actions><label class=\"switch\"><input type=checkbox name=cal_enabled_" + std::to_string(i) + (c.enabled?" checked":"") + "><span class=\"slider\"></span></label></div>";
        html += "<div class=meta><div class=title>" + (c.summary.empty()?"(Unnamed)":c.summary) + "</div>";
        if(!c.description.empty()) {
            html += "<div class=desc>" + c.description + "</div>";
        }
        if(c.primary) {
            html += "<div class=primary>Primary calendar</div>";
        }
        html += "<div class=id>ID: " + c.calendarId + "</div></div>";
        html += "</div>";
    }
    html += "</div>";

    // Display settings
    html += "<div class=controls>";
    html += "<div class=panel>";
    html += "<h2 style='margin:0 0 10px 0;font-size:15px'>Display</h2>";
    html += "<label class=field>Primary color<input type=color name=primaryColor value='" + primaryHex + "'></label>";
    html += "<label class=field>Background color<input type=color name=bgColor value='" + bgHex + "'></label>";
    html += "</div>";

    // Buttons
    html += "<div class=panel>";
    html += "<h2 style='margin:0 0 10px 0;font-size:15px'>Actions</h2>";
    html += "<div class=btn-row><button type=submit>Save</button> <button class=secondary name=refresh value=1 formaction='/refresh' formmethod='post'>Refresh calendars</button></div>";
    html += "</div>";

    html += "</div>"; // controls

    html += "</form>";
    html += "</main>";

    html += "</body></html>";
    return html;
}
