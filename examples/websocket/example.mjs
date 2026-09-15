// Node 18+ / Electron. Start foobar2000 with foo_zero_bus first.
//   node examples/websocket/example.mjs

const url = process.env.FOO_ZERO_BUS_WS_URL ?? "ws://127.0.0.1:17890";

const envelope = (type, receiver, payload, msgId = "", correlationId = "") =>
  JSON.stringify({
    sender: "",
    receiver,
    type,
    msg_id: msgId,
    correlation_id: correlationId,
    payload,
  });

const socket = new WebSocket(url);

socket.addEventListener("open", () => {
  const request = envelope(
    1,
    "plugin.localize",
    JSON.stringify({ text: "Hello", to: "zh-CN" }),
    "req_node_1"
  );
  console.log(">>", request);
  socket.send(request);
});

socket.addEventListener("message", (event) => {
  console.log("<<", event.data);
  const frame = JSON.parse(event.data);
  if (frame.type === 2 || frame.type === 5) {
    socket.close();
  }
});

socket.addEventListener("error", (error) => {
  console.error(error);
});
